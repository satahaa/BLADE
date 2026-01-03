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
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        console.log('=== LOGIN ATTEMPT ===');
        console.log('Entered username:', username);
        console.log('Entered password:', password);
        console.log('Expected username:', this.authConfig?.username);
        console.log('Expected password:', this.authConfig?.password);
        console.log('Auth enabled:', this.authConfig?.authEnabled);

        // Check against server-provided credentials
        if (this.authConfig && this.authConfig.authEnabled) {
            // Exact string comparison
            const usernameMatch = username === this.authConfig.username;
            const passwordMatch = password === this.authConfig.password;

            console.log('Username match:', usernameMatch);
            console.log('Password match:', passwordMatch);

            if (usernameMatch && passwordMatch) {
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
            <div class="file-item">
                <div class="file-info">
                    <img src="${this.getFileIcon(file.name)}" alt="" style="width: 20px; height: 20px; margin-right: 8px; vertical-align: middle;">
                    <span class="file-name">${file.name}</span>
                    <span class="file-size">${this.formatFileSize(file.size)}</span>
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

    sendFiles() {
        if (this.selectedFiles.length === 0) {
            this.showNotification('No files selected', 'error');
            return;
        }

        // Simulate file sending
        this.showNotification(`Sending ${this.selectedFiles.length} file(s)...`, 'info');

        setTimeout(() => {
            this.showNotification('Files sent successfully!', 'success');
            this.selectedFiles = [];
            this.updateSelectedFilesDisplay();
        }, 2000);
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
