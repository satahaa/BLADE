// BLADE Web Interface JavaScript

class BLADEClient {
    constructor() {
        this.authenticated = false;
        this.sessionStartTime = null;
        this.sessionTimer = null;
        this.init();
    }

    init() {
        // Setup event listeners
        this.setupEventListeners();
        
        // Check authentication status
        this.checkAuthStatus();
        
        // Update UI
        this.updateStatus('connecting');
        
        // Simulate connection (in a real implementation, this would connect to the server)
        setTimeout(() => {
            this.updateStatus('connected');
            this.loadServerInfo();
        }, 1000);
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

        // Logout button
        const logoutBtn = document.getElementById('logoutBtn');
        if (logoutBtn) {
            logoutBtn.addEventListener('click', () => {
                this.handleLogout();
            });
        }
    }

    checkAuthStatus() {
        // In a real implementation, check if user has a valid token
        const token = localStorage.getItem('blade_token');
        if (token) {
            this.authenticated = true;
            this.showDashboard();
        }
    }

    handleLogin() {
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        // Simulate authentication (in production, this would call the server)
        if (username === 'admin' && password === 'admin123') {
            const token = this.generateToken();
            localStorage.setItem('blade_token', token);
            this.authenticated = true;
            this.showDashboard();
            this.showNotification('Login successful!', 'success');
        } else {
            this.showNotification('Invalid credentials', 'error');
        }
    }

    handleLogout() {
        localStorage.removeItem('blade_token');
        this.authenticated = false;
        this.hideMainContent();
        document.getElementById('authSection').style.display = 'block';
        if (this.sessionTimer) {
            clearInterval(this.sessionTimer);
        }
        this.showNotification('Logged out successfully', 'success');
    }

    showDashboard() {
        document.getElementById('authSection').style.display = 'none';
        document.getElementById('dashboard').style.display = 'block';
        this.sessionStartTime = Date.now();
        this.startSessionTimer();
        this.loadDevices();
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
        document.getElementById('serverPort').textContent = window.location.port || '8081';
        this.updateConnectedUsers();
    }

    updateConnectedUsers() {
        // Simulate connected users count
        const count = Math.floor(Math.random() * 5) + 1;
        document.getElementById('connectedUsers').textContent = count;
    }

    startSessionTimer() {
        this.sessionTimer = setInterval(() => {
            const elapsed = Date.now() - this.sessionStartTime;
            const hours = Math.floor(elapsed / 3600000);
            const minutes = Math.floor((elapsed % 3600000) / 60000);
            const seconds = Math.floor((elapsed % 60000) / 1000);
            
            const timeString = `${String(hours).padStart(2, '0')}:${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
            document.getElementById('sessionTime').textContent = timeString;
        }, 1000);
    }

    loadDevices() {
        const deviceList = document.getElementById('deviceList');
        
        // Simulate some connected devices
        const devices = [
            { name: 'Desktop PC', ip: '192.168.1.100', status: 'online' },
            { name: 'Laptop', ip: '192.168.1.101', status: 'online' },
            { name: 'Mobile Phone', ip: '192.168.1.102', status: 'online' }
        ];

        if (devices.length === 0) {
            deviceList.innerHTML = '<p class="no-devices">No devices connected</p>';
            return;
        }

        deviceList.innerHTML = devices.map(device => `
            <div class="device-item">
                <div class="device-info">
                    <span class="device-name">${device.name}</span>
                    <span class="device-ip">${device.ip}</span>
                </div>
                <span class="device-status" style="color: var(--success-color);">Online</span>
            </div>
        `).join('');
    }

    handleFileSelection(event) {
        const files = event.target.files;
        const selectedFilesDiv = document.getElementById('selectedFiles');
        const sendBtn = document.getElementById('sendBtn');

        if (files.length === 0) {
            selectedFilesDiv.innerHTML = '';
            sendBtn.disabled = true;
            return;
        }

        selectedFilesDiv.innerHTML = Array.from(files).map(file => `
            <div class="file-item">
                <span class="file-name">${file.name}</span>
                <span class="file-size">${this.formatFileSize(file.size)}</span>
            </div>
        `).join('');

        sendBtn.disabled = false;
    }

    sendFiles() {
        const fileInput = document.getElementById('fileInput');
        const files = fileInput.files;

        if (files.length === 0) {
            this.showNotification('No files selected', 'error');
            return;
        }

        // Simulate file sending
        this.showNotification(`Sending ${files.length} file(s)...`, 'info');
        
        setTimeout(() => {
            this.showNotification('Files sent successfully!', 'success');
            fileInput.value = '';
            document.getElementById('selectedFiles').innerHTML = '';
            document.getElementById('sendBtn').disabled = true;
        }, 2000);
    }

    formatFileSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        const k = 1024;
        const sizes = ['Bytes', 'KB', 'MB', 'GB'];
        const i = Math.floor(Math.log(bytes) / Math.log(k));
        return Math.round(bytes / Math.pow(k, i) * 100) / 100 + ' ' + sizes[i];
    }

    generateToken() {
        return Math.random().toString(36).substring(2) + Date.now().toString(36);
    }

    showNotification(message, type = 'info') {
        // Create a simple notification
        const notification = document.createElement('div');
        notification.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 15px 25px;
            border-radius: 5px;
            color: white;
            font-weight: 600;
            z-index: 1000;
            animation: slideIn 0.3s ease-out;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
        `;

        if (type === 'success') {
            notification.style.background = 'var(--success-color)';
        } else if (type === 'error') {
            notification.style.background = 'var(--danger-color)';
        } else {
            notification.style.background = 'var(--primary-color)';
        }

        notification.textContent = message;
        document.body.appendChild(notification);

        setTimeout(() => {
            notification.style.animation = 'slideOut 0.3s ease-in';
            setTimeout(() => {
                document.body.removeChild(notification);
            }, 300);
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
    new BLADEClient();
});
