# Authentication Configuration

## Overview

BLADE now supports dynamic authentication configuration. The web interface automatically adapts based on whether authentication is enabled on the server.

## How It Works

### 1. Server-Side Configuration

The server passes authentication configuration to the HTTP server:
- **No Auth Mode** (default): Run `blade.exe` without any arguments
- **Auth Mode**: Run `blade.exe -u <username> -p <password>`

### 2. Web Interface Behavior

The JavaScript client fetches authentication configuration from `/api/auth-config` endpoint:

```json
// No Auth Mode
{
  "authEnabled": false
}

// Auth Mode
{
  "authEnabled": true,
  "username": "admin",
  "password": "secret"
}
```

### 3. Dynamic UI

#### No Auth Mode (Default)
- Login page is **hidden**
- Web interface loads **directly** to the dashboard
- Logout button is **hidden**
- Users can access files immediately

#### Auth Mode (When -u and -p provided)
- Login page is **shown**
- User must enter credentials matching those provided via command line
- Logout button is **visible**
- Session management is active

## Usage Examples

### Running without authentication (default):
```bash
blade.exe
```

Web interface will:
- Skip the login page
- Load directly to the dashboard
- No credentials required

### Running with authentication:
```bash
blade.exe -u admin -p mypassword
```

Web interface will:
- Show a login page
- Require username: `admin` and password: `mypassword`
- Display logout button once authenticated

## Security Notes

1. **No hardcoded credentials**: The JavaScript no longer has hardcoded username/password
2. **Server-driven auth**: Authentication is controlled entirely by the server
3. **HTTPS only**: Server always runs on HTTPS (port 443) for secure communication
4. **Requires Admin**: Port 443 requires administrator privileges on Windows

## Technical Details

### Modified Files

1. **HTTPServer.h / HTTPServer.cpp**
   - Added constructor parameters: `useAuth`, `username`, `password`
   - Added `/api/auth-config` endpoint to serve auth configuration
   - Auth config is sent as JSON to the client

2. **script.js**
   - Removed hardcoded credentials
   - Added `loadAuthConfig()` method to fetch server config
   - Dynamic login page visibility based on `authEnabled` flag
   - Credentials validation uses server-provided values

3. **index.html**
   - Auth section hidden by default (`display: none`)
   - Shown only when auth is enabled

4. **main.cpp**
   - Updated help text to clarify authentication behavior
   - Default is no authentication
   - Authentication enabled only when both -u and -p are provided

## API Endpoint

### GET `/api/auth-config`

Returns authentication configuration in JSON format.

**Response (No Auth):**
```json
{
  "authEnabled": false
}
```

**Response (Auth Enabled):**
```json
{
  "authEnabled": true,
  "username": "admin",
  "password": "secret"
}
```

## Best Practices

1. **Development**: Use no-auth mode for quick testing
   ```bash
   blade.exe
   ```

2. **Production/Shared Networks**: Use auth mode for security
   ```bash
   blade.exe -u admin -p strongPassword123
   ```

3. **Admin Privileges**: Always run with administrator privileges to bind to port 443

4. **HTTPS**: The server automatically uses HTTPS with a self-signed certificate. Browsers will show a security warning - this is expected for local development.

