// BLADE Web Interface JavaScript

class BladeApp {
    constructor() {
        this.authenticated = false;
        this.authConfig = null;
        this.sessionStartTime = null;
        this.sessionTimer = null;
        this.selectedFiles = []; // Store selected files persistently
        this.heartbeatInterval = null;
        this.reconnectInterval = null;
        this.pendingFilesInterval = null;
        this.downloadedFiles = new Set(); // Track already downloaded files using name+size as key
        this.receivedFileElements = new Map(); // Track DOM elements by name+size to prevent duplicates
        this.isDownloading = false; // Flag to prevent concurrent download checks
        this.isReconnecting = false;
        this.heartbeatFailCount = 0;
        this.maxHeartbeatFails = 3; // Increase to 3 to avoid false disconnections
        this.init();
    }

    init() {
        // Setup event listeners
        this.setupEventListeners();

        // Restore session state if exists
        this.restoreSessionState();

        // Check authentication status and load auth config
        this.loadAuthConfig();
    }

    saveSessionState() {
        if (this.authenticated) {
            const state = {
                authenticated: true,
                sessionStartTime: this.sessionStartTime,
                selectedFiles: this.selectedFiles.map(f => ({
                    name: f.name,
                    size: f.size,
                    type: f.type
                }))
            };
            localStorage.setItem('bladeSession', JSON.stringify(state));
        }
    }

    restoreSessionState() {
        const savedState = localStorage.getItem('bladeSession');
        if (savedState) {
            try {
                const state = JSON.parse(savedState);
                // We'll restore authentication after server responds
                if (state.sessionStartTime) {
                    this.sessionStartTime = state.sessionStartTime;
                }
                // Note: We can't restore actual File objects, but we keep the list for display
                console.log('Session state found, will attempt to restore after reconnection');
            } catch (error) {
                console.error('Failed to restore session state:', error);
                localStorage.removeItem('bladeSession');
            }
        }
    }

    clearSessionState() {
        localStorage.removeItem('bladeSession');
    }

    startHeartbeat() {
        // Clear any existing heartbeat
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
        }

        this.heartbeatFailCount = 0;

        // Check server connection every 3 seconds (increased from 2)
        this.heartbeatInterval = setInterval(async () => {
            try {
                const controller = new AbortController();
                const timeoutId = setTimeout(() => controller.abort(), 3000); // Increased timeout

                const response = await fetch('/api/heartbeat?t=' + Date.now(), {
                    method: 'GET',
                    headers: {
                        'Cache-Control': 'no-cache'
                    },
                    signal: controller.signal
                });

                clearTimeout(timeoutId);

                if (response.ok) {
                    this.heartbeatFailCount = 0;
                    if (this.isReconnecting) {
                        // Server is back online
                        this.handleReconnection();
                    }
                } else {
                    this.handleHeartbeatFailure();
                }
            } catch (error) {
                this.handleHeartbeatFailure();
            }
        }, 3000); // Increased from 2000
    }

    handleHeartbeatFailure() {
        this.heartbeatFailCount++;

        // After 3 consecutive failures, consider disconnected (increased from 2)
        if (this.heartbeatFailCount >= this.maxHeartbeatFails && !this.isReconnecting) {
            console.log('Server disconnected after', this.heartbeatFailCount, 'failed heartbeats');
            this.handleDisconnection();
        }
    }

    handleDisconnection() {
        this.isReconnecting = true;
        this.updateStatus('disconnected');
        this.saveSessionState();

        // No notification - silent reconnection attempt

        // Stop heartbeat during reconnection
        if (this.heartbeatInterval) {
            clearInterval(this.heartbeatInterval);
            this.heartbeatInterval = null;
        }

        // Stop pending files polling during disconnection
        if (this.pendingFilesInterval) {
            clearInterval(this.pendingFilesInterval);
            this.pendingFilesInterval = null;
        }

        // Start reconnection attempts
        this.attemptReconnection();
    }

    async attemptReconnection() {
        if (this.reconnectInterval) {
            clearInterval(this.reconnectInterval);
        }

        // Try to reconnect every 5 seconds
        this.reconnectInterval = setInterval(async () => {
            try {
                const controller = new AbortController();
                const timeoutId = setTimeout(() => controller.abort(), 4000);

                const response = await fetch('/api/heartbeat?t=' + Date.now(), {
                    method: 'GET',
                    headers: {
                        'Cache-Control': 'no-cache'
                    },
                    signal: controller.signal
                });

                clearTimeout(timeoutId);

                if (response.ok) {
                    // Server is back online
                    this.handleReconnection();
                }
            } catch (error) {
                // Still disconnected, keep trying silently
                console.log('Reconnection attempt failed, retrying...');
            }
        }, 5000);
    }

    handleReconnection() {
        console.log('Server reconnected!');
        this.isReconnecting = false;
        this.heartbeatFailCount = 0;

        // Clear reconnection interval
        if (this.reconnectInterval) {
            clearInterval(this.reconnectInterval);
            this.reconnectInterval = null;
        }

        // Reload auth config and restore session
        this.loadAuthConfig().then(() => {
            const savedState = localStorage.getItem('bladeSession');
            if (savedState && this.authenticated) {
                this.updateStatus('connected');
                // No notification - silent reconnection
                this.loadServerInfo();

                // Resume session timer if it was running
                if (this.sessionStartTime && !this.sessionTimer) {
                    this.startSessionTimer();
                }
            }

            // Restart heartbeat
            this.startHeartbeat();
        });
    }

    async loadAuthConfig() {
        // Only update status to connecting if we're not already connected
        if (!this.authenticated) {
            this.updateStatus('connecting');
        }

        try {
            // Add timeout to prevent hanging
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000);

            const response = await fetch('/api/auth-config?t=' + Date.now(), {
                method: 'GET',
                headers: {
                    'Cache-Control': 'no-cache, no-store, must-revalidate',
                    'Pragma': 'no-cache',
                    'Expires': '0'
                },
                signal: controller.signal
            });

            clearTimeout(timeoutId);

            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }

            const text = await response.text();
            console.log('Raw response:', text);

            this.authConfig = JSON.parse(text);
            console.log('Auth config received:', this.authConfig);

            // If authentication is not enabled, bypass login
            if (!this.authConfig.authEnabled) {
                console.log('Auth disabled - bypassing login');
                this.authenticated = true;
                this.updateStatus('connected');
                this.loadServerInfo();
                this.showDashboard();
                this.startHeartbeat(); // Start monitoring connection
            } else {
                // Authentication is enabled - always show login page
                // Clear any stale sessions when auth is required
                this.clearSessionState();
                console.log('Auth enabled - showing login page');
                this.updateStatus('disconnected'); // Not connected until login
                document.getElementById('authSection').style.display = 'block';
            }
        } catch (error) {
            console.error('Failed to load auth config:', error);
            console.error('Error details:', error.name, error.message);

            // Check if it's a timeout or network error
            if (error.name === 'AbortError') {
                this.updateStatus('disconnected');
                // No notification
            } else {
                this.updateStatus('disconnected');
                // No notification
            }

            // If we were previously connected, try to reconnect
            const savedState = localStorage.getItem('bladeSession');
            if (savedState && !this.isReconnecting) {
                this.isReconnecting = true;
                this.attemptReconnection();
            } else {
                // Show error message on page
                const authSection = document.getElementById('authSection');
                if (authSection) {
                    authSection.style.display = 'block';
                    authSection.innerHTML = `
                        <div class="card">
                            <h2>Connection Error</h2>
                            <p style="color: var(--danger-color); margin-bottom: 15px;">
                                Failed to connect to the server.
                            </p>
                            <p style="color: var(--text-secondary); margin-bottom: 15px;">
                                Error: ${error.message}
                            </p>
                            <p style="color: var(--text-secondary);">
                                Please ensure the BLADE server is running and try refreshing the page.
                            </p>
                            <button class="btn btn-primary" onclick="location.reload()" style="margin-top: 20px;">
                                Retry Connection
                            </button>
                        </div>
                    `;
                }
            }
        }
    }

    setupEventListeners() {
        // Login form
        const loginForm = document.getElementById('loginForm');
        if (loginForm) {
            loginForm.addEventListener('submit', (e) => {
                e.preventDefault();
                this.handleLogin();
            });
        }

        // File input
        const fileInput = document.getElementById('fileInput');
        if (fileInput) {
            fileInput.addEventListener('change', (e) => {
                this.handleFileSelection(e);
            });
        }

        // Send button
        const sendBtn = document.getElementById('sendBtn');
        if (sendBtn) {
            sendBtn.addEventListener('click', () => {
                this.sendFiles();
            });
        }
    }

    handleLogin() {
        const password = document.getElementById('password').value;

        console.log('=== LOGIN ATTEMPT ===');
        console.log('Entered password:', password);
        console.log('Expected password:', this.authConfig?.password);
        console.log('Auth enabled:', this.authConfig?.authEnabled);

        // Check against server-provided credentials
        if (this.authConfig && this.authConfig.authEnabled) {
            // Exact string comparison
            const passwordMatch = password === this.authConfig.password;

            console.log('Password match:', passwordMatch);

            if (passwordMatch) {
                console.log('Login successful!');
                this.authenticated = true;
                this.updateStatus('connected');
                this.loadServerInfo();
                this.showDashboard();
                this.saveSessionState(); // Save session after successful login
                this.startHeartbeat(); // Start monitoring connection
            } else {
                console.log('Login failed - Invalid credentials');
                this.showNotification('Invalid credentials', 'error');
            }
        } else {
            console.log('Authentication is not enabled');
            this.showNotification('Authentication is not enabled', 'error');
        }
    }

    showDashboard() {
        document.getElementById('authSection').style.display = 'none';
        document.getElementById('dashboard').style.display = 'block';

        // Hide logout button entirely - logout not allowed
        const logoutBtn = document.getElementById('logoutBtn');
        if (logoutBtn) {
            logoutBtn.style.display = 'none';
        }

        if (!this.sessionStartTime) {
            this.sessionStartTime = Date.now();
        }

        if (!this.sessionTimer) {
            this.startSessionTimer();
        }

        // Start polling for pending files from server
        this.startPendingFilesPolling();
    }

    startPendingFilesPolling() {
        // Clear any existing interval
        if (this.pendingFilesInterval) {
            clearInterval(this.pendingFilesInterval);
        }

        // Poll every 2 seconds
        this.pendingFilesInterval = setInterval(() => {
            this.checkPendingFiles();
        }, 2000);

        // Also check immediately
        this.checkPendingFiles();
    }

    async checkPendingFiles() {
        // Skip if already downloading
        if (this.isDownloading) {
            return;
        }

        try {
            const response = await fetch('/api/pending-files?t=' + Date.now(), {
                method: 'GET',
                headers: {
                    'Cache-Control': 'no-cache'
                }
            });

            if (response.ok) {
                const data = await response.json();
                const files = data.files || [];

                // Filter out already downloaded files using name+size as unique key
                const newFiles = files.filter(file => {
                    const fileKey = `${file.name}-${file.size}`;
                    return !this.downloadedFiles.has(fileKey);
                });

                // If there are new files, handle them
                if (newFiles.length > 0) {
                    // Detect iOS Safari
                    const isIOS = /iPad|iPhone|iPod/.test(navigator.userAgent) && !window.MSStream;

                    if (isIOS && newFiles.length > 1) {
                        // On iOS with multiple files, show download buttons instead of auto-download
                        this.showPendingFilesForManualDownload(newFiles);
                    } else {
                        // On other platforms or single file, auto-download
                        this.isDownloading = true;
                        await this.downloadFilesSequentially(newFiles);
                        this.isDownloading = false;
                    }
                }
            }
        } catch (error) {
            console.error('Failed to check pending files:', error);
            this.isDownloading = false;
        }
    }

    showPendingFilesForManualDownload(files) {
        // Add each file to received files with a download button
        for (const file of files) {
            const fileKey = `${file.name}-${file.size}`;

            // Skip if already shown (check both sets)
            if (this.downloadedFiles.has(fileKey) || this.receivedFileElements.has(fileKey)) {
                continue;
            }

            // Mark as shown (not downloaded yet)
            this.downloadedFiles.add(fileKey);

            // Add to UI with download button
            this.addReceivedFileWithDownloadButton(file);
        }

        // Show notification
        this.showNotification(`${files.length} files ready to download. Tap each to download.`, 'info');
    }

    addReceivedFileWithDownloadButton(file) {
        const receivedFilesDiv = document.getElementById('receivedFiles');
        const fileKey = `${file.name}-${file.size}`;

        // Check if already in DOM (prevent duplicates)
        if (this.receivedFileElements.has(fileKey)) {
            return;
        }

        // Remove "No files received" message if present
        const noFilesMsg = receivedFilesDiv.querySelector('.no-files');
        if (noFilesMsg) {
            noFilesMsg.remove();
        }

        // Use filename in URL to preserve original extension
        const downloadUrl = `/api/download/${file.index}/${encodeURIComponent(file.name)}`;

        // Add file item with download button
        const fileItem = document.createElement('div');
        fileItem.className = 'file-item';
        fileItem.innerHTML = `
            <div class="file-info" style="flex: 1;">
                <img src="${this.getFileIcon(file.name)}" alt="" style="width: 20px; height: 20px; margin-right: 8px; vertical-align: middle;">
                <span class="file-name">${file.name}</span>
                <span class="file-size">${this.formatFileSize(file.size)}</span>
                <progress class="file-progress" value="0" max="100" style="width: 100%; height: 6px; display: none;"></progress>
            </div>
            <a href="${downloadUrl}" download="${file.name}" class="btn btn-primary" style="padding: 8px 16px; font-size: 14px; text-decoration: none;">
                Download
            </a>
        `;
        receivedFilesDiv.appendChild(fileItem);

        // Track this element
        this.receivedFileElements.set(fileKey, fileItem);
    }

    async downloadFilesSequentially(files) {
        for (let i = 0; i < files.length; i++) {
            const file = files[i];
            const fileKey = `${file.name}-${file.size}`;

            // Skip if already downloaded (double check)
            if (this.downloadedFiles.has(fileKey)) {
                continue;
            }

            // Mark as downloaded
            this.downloadedFiles.add(fileKey);

            // Download this file with progress tracking
            await this.downloadFileFromServer(file);

            // Wait for server to process the download completion
            if (i < files.length - 1) {
                await new Promise(resolve => setTimeout(resolve, 1000));

                // Re-check pending files to get updated indices after file removal
                // This prevents convoy effect where indices shift after downloads
                try {
                    const response = await fetch('/api/pending-files?t=' + Date.now(), {
                        method: 'GET',
                        headers: { 'Cache-Control': 'no-cache' }
                    });

                    if (response.ok) {
                        const data = await response.json();
                        const remainingFiles = data.files || [];

                        // Update the files array with fresh data
                        // Match remaining files by name and size
                        const updatedFiles = [];
                        for (let j = i + 1; j < files.length; j++) {
                            const originalFile = files[j];
                            const matchingFile = remainingFiles.find(rf =>
                                rf.name === originalFile.name && rf.size === originalFile.size
                            );
                            if (matchingFile) {
                                updatedFiles.push(matchingFile);
                            }
                        }

                        // Replace remaining files with updated indices
                        files.splice(i + 1, files.length - i - 1, ...updatedFiles);
                    }
                } catch (error) {
                    console.error('Failed to refresh pending files:', error);
                }
            }
        }
    }

    async downloadFileFromServer(file) {
        console.log('Downloading file from server:', file.name, 'size:', file.size);
        this.showNotification(`Downloading: ${file.name}`, 'info');

        // Add to received files display with progress bar
        this.addReceivedFile(file);

        // Use the filename endpoint to preserve original extension
        const downloadUrl = `/api/download/${file.index}/${encodeURIComponent(file.name)}`;

        return new Promise((resolve, reject) => {
            const xhr = new XMLHttpRequest();
            xhr.open('GET', downloadUrl, true);
            xhr.responseType = 'blob';

            const fileKey = `${file.name}-${file.size}`;

            // Track download progress
            xhr.onprogress = (event) => {
                if (event.lengthComputable) {
                    const percentComplete = Math.round((event.loaded / event.total) * 100);
                    this.updateReceivedFileProgress(fileKey, percentComplete);
                }
            };

            xhr.onload = () => {
                if (xhr.status === 200) {
                    // Create a download link for the blob
                    const blob = xhr.response;
                    const url = window.URL.createObjectURL(blob);
                    const link = document.createElement('a');
                    link.href = url;
                    link.download = file.name;
                    link.style.display = 'none';
                    document.body.appendChild(link);
                    link.click();

                    // Clean up
                    setTimeout(() => {
                        document.body.removeChild(link);
                        window.URL.revokeObjectURL(url);
                    }, 100);

                    // Set progress to 100%
                    this.updateReceivedFileProgress(fileKey, 100);
                    resolve();
                } else {
                    this.showNotification(`Failed to download: ${file.name}`, 'error');
                    reject(new Error(`Download failed with status ${xhr.status}`));
                }
            };

            xhr.onerror = () => {
                this.showNotification(`Download error: ${file.name}`, 'error');
                reject(new Error('Network error'));
            };

            xhr.send();
        });
    }

    updateReceivedFileProgress(fileKey, percent) {
        const fileElement = this.receivedFileElements.get(fileKey);
        if (fileElement) {
            const progressBar = fileElement.querySelector('.file-progress');
            if (progressBar) {
                progressBar.style.display = 'block';
                progressBar.value = percent;
            }
        }
    }

    addReceivedFile(file) {
        const receivedFilesDiv = document.getElementById('receivedFiles');
        const fileKey = `${file.name}-${file.size}`;

        // Check if this file is already displayed (prevent duplicates in UI)
        if (this.receivedFileElements.has(fileKey)) {
            return; // Already in the UI
        }

        // Remove "No files received" message if present
        const noFilesMsg = receivedFilesDiv.querySelector('.no-files');
        if (noFilesMsg) {
            noFilesMsg.remove();
        }

        // Add file item with unique ID and progress bar
        const fileItem = document.createElement('div');
        fileItem.className = 'file-item';
        fileItem.innerHTML = `
            <div class="file-info">
                <div style="display: flex; align-items: center; gap: 8px;">
                    <img src="${this.getFileIcon(file.name)}" alt="" style="width: 20px; height: 20px; flex-shrink: 0;">
                    <span class="file-name">${file.name}</span>
                </div>
                <span class="file-size">${this.formatFileSize(file.size)}</span>
                <progress class="file-progress" value="0" max="100" style="width: 100%; height: 6px; margin-top: 6px;"></progress>
            </div>
        `;
        receivedFilesDiv.appendChild(fileItem);

        // Track this element
        this.receivedFileElements.set(fileKey, fileItem);
    }

    hideMainContent() {
        document.getElementById('dashboard').style.display = 'none';
    }

    updateStatus(status) {
        const statusIndicator = document.getElementById('statusIndicator');
        const statusText = document.getElementById('statusText');

        if (status === 'connected') {
            statusIndicator.classList.add('connected');
            statusText.textContent = 'Connected';
        } else if (status === 'connecting') {
            statusIndicator.classList.remove('connected');
            statusText.textContent = 'Connecting...';
        } else {
            statusIndicator.classList.remove('connected');
            statusText.textContent = 'Disconnected';
        }
    }

    loadServerInfo() {
        // In a real implementation, fetch this from the server
        document.getElementById('serverIP').textContent = window.location.hostname || 'localhost';
        document.getElementById('serverPort').textContent = window.location.port || '80';
        this.loadConnectedDevices();
    }

    async loadConnectedDevices() {
        const devicesList = document.getElementById('connectedDevicesList');

        try {
            // Fetch connected devices from server
            const response = await fetch('/api/connected-devices?t=' + Date.now(), {
                method: 'GET',
                headers: {
                    'Cache-Control': 'no-cache, no-store, must-revalidate'
                }
            });

            if (response.ok) {
                const data = await response.json();
                const connectedIPs = data.devices || [];

                if (connectedIPs.length === 0) {
                    devicesList.innerHTML = '<p style="color: #888; font-style: italic;">No devices connected</p>';
                } else {
                    devicesList.innerHTML = connectedIPs.map(ip =>
                        `<div style="padding: 8px; background: #f5f5f5; border-radius: 4px; color: #333; margin-bottom: 5px; display: flex; align-items: center; gap: 8px;">
                            <img src="icons/devices.svg" alt="" style="width: 20px; height: 20px;">
                            <strong>${ip}</strong>
                        </div>`
                    ).join('');
                }
            } else {
                devicesList.innerHTML = '<p style="color: #888; font-style: italic;">No devices connected</p>';
            }
        } catch (error) {
            console.error('Failed to fetch connected devices:', error);
            devicesList.innerHTML = '<p style="color: #888; font-style: italic;">No devices connected</p>';
        }

        // Poll for updates every 3 seconds
        setTimeout(() => this.loadConnectedDevices(), 3000);
    }

    startSessionTimer() {
        if (this.sessionTimer) {
            clearInterval(this.sessionTimer);
        }

        this.sessionTimer = setInterval(() => {
            if (!this.sessionStartTime) return;

            const elapsed = Date.now() - this.sessionStartTime;
            const hours = Math.floor(elapsed / 3600000);
            const minutes = Math.floor((elapsed % 3600000) / 60000);
            const seconds = Math.floor((elapsed % 60000) / 1000);

            const sessionTimeElement = document.getElementById('sessionTime');
            if (sessionTimeElement) {
                sessionTimeElement.textContent = `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
            }
        }, 1000);
    }


    handleFileSelection(event) {
        const files = event.target.files;

        // Add newly selected files to our persistent list (avoid duplicates by name)
        Array.from(files).forEach(file => {
            const exists = this.selectedFiles.some(f => f.name === file.name && f.size === file.size);
            if (!exists) {
                this.selectedFiles.push(file);
            }
        });

        // Clear the file input so user can select the same files again if needed
        event.target.value = '';

        this.updateSelectedFilesDisplay();
    }

    updateSelectedFilesDisplay() {
        const selectedFilesDiv = document.getElementById('selectedFiles');
        const sendBtn = document.getElementById('sendBtn');

        if (this.selectedFiles.length === 0) {
            selectedFilesDiv.innerHTML = '';
            sendBtn.disabled = true;
            return;
        }

        selectedFilesDiv.innerHTML = this.selectedFiles.map((file, index) => `
            <div class="file-item" style="margin-bottom: 16px;">
                <div class="file-info">
                    <img src="${this.getFileIcon(file.name)}" alt="" style="width: 20px; height: 20px; margin-right: 8px; vertical-align: middle;">
                    <span class="file-name">${file.name}</span>
                    <span class="file-size">${this.formatFileSize(file.size)}</span>
                    <progress id="progress-${index}" value="0" max="100" class="file-progress"></progress>
                </div>
                <button class="remove-file-btn" onclick="app.removeFile(${index})" aria-label="Remove file"><img src="icons/delete.svg" alt="Delete" style="width: 20px; height: 20px;"></button>
            </div>
        `).join('');

        sendBtn.disabled = false;
    }
    getFileIcon(filename) {
        const ext = filename.toLowerCase().split('.').pop();

        // Image files
        if (['jpg', 'jpeg', 'png', 'gif', 'bmp', 'svg', 'webp', 'ico'].includes(ext)) {
            return 'icons/image.svg';
        }
        // Video files
        if (['mp4', 'avi', 'mkv', 'mov', 'wmv', 'flv', 'webm', 'm4v'].includes(ext)) {
            return 'icons/video.svg';
        }
        // Audio files
        if (['mp3', 'wav', 'ogg', 'flac', 'aac', 'm4a', 'wma'].includes(ext)) {
            return 'icons/audio.svg';
        }
        // Source code files
        if (['js', 'ts', 'jsx', 'tsx', 'cpp', 'c', 'h', 'hpp', 'py', 'java', 'cs', 'php', 'rb', 'go', 'rs', 'swift', 'kt', 'html', 'css', 'scss', 'json', 'xml', 'yaml', 'yml', 'sql', 'sh', 'bat', 'ps1'].includes(ext)) {
            return 'icons/source_code.svg';
        }
        // Text/document files (default)
        return 'icons/text.svg';
    }

    removeFile(index) {
        this.selectedFiles.splice(index, 1);
        this.updateSelectedFilesDisplay();
    }

    togglePasswordVisibility() {
        const passwordInput = document.getElementById('password');
        const eyeIcon = document.querySelector('.eye-icon img');

        if (passwordInput.type === 'password') {
            passwordInput.type = 'text';
            eyeIcon.src = 'icons/not_visible.svg';
            eyeIcon.alt = 'Hide';
        } else {
            passwordInput.type = 'password';
            eyeIcon.src = 'icons/visible.svg';
            eyeIcon.alt = 'Show';
        }
    }

    async sendFiles() {
        if (this.selectedFiles.length === 0) {
            this.showNotification('No files selected', 'error');
            return;
        }

        for (let i = 0; i < this.selectedFiles.length; i++) {
            const file = this.selectedFiles[i];

            // Announce the file to server first (so it appears in UI immediately)
            try {
                await fetch('/api/upload/announce', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify({
                        filename: file.name,
                        size: file.size
                    })
                });
            } catch (e) {
                console.error('Failed to announce file:', e);
            }

            const form = new FormData();
            form.append('files[]', file, file.name);

            await new Promise((resolve) => {
                const xhr = new XMLHttpRequest();
                xhr.open('POST', '/api/upload', true);

                xhr.upload.onprogress = (e) => {
                    if (e.lengthComputable) {
                        const percent = Math.round((e.loaded / e.total) * 100);
                        const progressBar = document.getElementById(`progress-${i}`);
                        if (progressBar) progressBar.value = percent;
                    }
                };

                xhr.onload = () => {
                    const progressBar = document.getElementById(`progress-${i}`);
                    if (progressBar) progressBar.value = 100;
                    resolve();
                };

                xhr.onerror = () => {
                    this.showNotification('Upload failed (network error)', 'error');
                    resolve();
                };

                xhr.send(form);
            });
        }

        this.showNotification('All files uploaded!', 'success');
        this.selectedFiles = [];
        this.updateSelectedFilesDisplay();
    }


    formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(1024));
        return `${(bytes / Math.pow(1024, i)).toFixed(2)} ${sizes[i]}`;
    }

    showNotification(message, type = 'info') {
        const notification = document.createElement('div');
        notification.className = `notification ${type}`;
        notification.textContent = message;
        document.body.appendChild(notification);

        setTimeout(() => {
            notification.style.animation = 'slideOut 0.3s ease-in';
            setTimeout(() => notification.remove(), 300);
        }, 3000);
    }
}
// Animations are now in style.css

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    window.app = new BladeApp();
});
