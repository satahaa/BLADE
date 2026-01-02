// BLADE Web Interface JavaScript

class BladeApp {
    constructor() {
        this.authenticated = false;
        this.authConfig = null;
        this.sessionStartTime = null;
        this.sessionTimer = null;
        this.selectedFiles = []; // Store selected files persistently
        this.init();
    }

    init() {
        // Setup event listeners
        this.setupEventListeners();
        
        // Check authentication status and load auth config
        this.loadAuthConfig();
    }

    async loadAuthConfig() {
        this.updateStatus('connecting');
        
        try {
            // Add timeout to prevent hanging
            const controller = new AbortController();
            const timeoutId = setTimeout(() => controller.abort(), 5000); // 5 second timeout

            // Add cache-busting parameter and proper headers for mobile
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
            } else {
                // Show login page - don't set status to connected yet
                console.log('Auth enabled - showing login page');
                console.log('Expected credentials:');
                console.log('  Username:', this.authConfig.username);
                console.log('  Password:', this.authConfig.password);
                this.updateStatus('disconnected'); // Not connected until login
                document.getElementById('authSection').style.display = 'block';
            }
        } catch (error) {
            console.error('Failed to load auth config:', error);
            console.error('Error details:', error.name, error.message);

            // Check if it's a timeout or network error
            if (error.name === 'AbortError') {
                this.updateStatus('disconnected');
                this.showNotification('Connection timeout. Please check if the server is running.', 'error');
            } else {
                this.updateStatus('disconnected');
                this.showNotification('Cannot connect to server. Please check your connection.', 'error');
            }

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
                console.log('✅ Login successful!');
                this.authenticated = true;
                this.updateStatus('connected');
                this.loadServerInfo();
                this.showDashboard();
                this.showNotification('Login successful!', 'success');
            } else {
                console.log('❌ Login failed - Invalid credentials');
                this.showNotification('Invalid credentials', 'error');
            }
        } else {
            console.log('❌ Authentication is not enabled');
            this.showNotification('Authentication is not enabled', 'error');
        }
    }

    handleLogout() {
        this.authenticated = false;
        this.hideMainContent();

        // Only show login page if authentication is enabled
        if (this.authConfig && this.authConfig.authEnabled) {
            document.getElementById('authSection').style.display = 'block';
            this.showNotification('Logged out successfully', 'success');
        } else {
            // If no auth, just reload the page to dashboard
            this.authenticated = true;
            this.showDashboard();
        }

        if (this.sessionTimer) {
            clearInterval(this.sessionTimer);
        }
    }

    showDashboard() {
        document.getElementById('authSection').style.display = 'none';
        document.getElementById('dashboard').style.display = 'block';

        // Hide logout button if authentication is disabled
        const logoutBtn = document.getElementById('logoutBtn');
        if (logoutBtn) {
            logoutBtn.style.display = 'none';
        }

        this.sessionStartTime = Date.now();
        this.startSessionTimer();
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
                        `<div style="padding: 8px; background: #f5f5f5; border-radius: 4px; color: #333; margin-bottom: 5px;">
                            <strong>📱 ${ip}</strong>
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
        this.sessionTimer = setInterval(() => {
            const elapsed = Date.now() - this.sessionStartTime;
            const hours = Math.floor(elapsed / 3600000);
            const minutes = Math.floor((elapsed % 3600000) / 60000);
            const seconds = Math.floor((elapsed % 60000) / 1000);

            document.getElementById('sessionTime').textContent = `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
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
                    <span class="file-name">${file.name}</span>
                    <span class="file-size">${this.formatFileSize(file.size)}</span>
                </div>
                <button class="remove-file-btn" onclick="app.removeFile(${index})" aria-label="Remove file"><img src="icons/delete.svg" alt="Delete" style="width: 20px; height: 20px;"></button>
            </div>
        `).join('');

        sendBtn.disabled = false;
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
        const colors = {
            success: 'var(--success-color)',
            error: 'var(--danger-color)',
            info: 'var(--primary-color)'
        };

        const notification = document.createElement('div');
        notification.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 25px;
            border-radius: 5px;
            background: ${colors[type] || colors.info};
            color: white;
            font-weight: 600;
            z-index: 1000;
            animation: slideIn 0.3s ease-out;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
        `;

        notification.textContent = message;
        document.body.appendChild(notification);

        setTimeout(() => {
            notification.style.animation = 'slideOut 0.3s ease-in';
            setTimeout(() => notification.remove(), 300);
        }, 3000);
    }
}

// Add CSS animations
const style = document.createElement('style');
style.textContent = `
    @keyframes slideIn {
        from {
            transform: translateX(400px);
            opacity: 0;
        }
        to {
            transform: translateX(0);
            opacity: 1;
        }
    }
    @keyframes slideOut {
        from {
            transform: translateX(0);
            opacity: 1;
        }
        to {
            transform: translateX(400px);
            opacity: 0;
        }
    }
`;
document.head.appendChild(style);

// Initialize the application
document.addEventListener('DOMContentLoaded', () => {
    window.app = new BladeApp();
});
