# BLADE GUI Visual Guide

This document provides a visual description of the BLADE GUI interface to help users understand what to expect.

## Application Window

### Window Properties
- **Title**: "BLADE - Local Network File Transfer"
- **Icon**: blade.ico (minimalist dagger design)
- **Size**: 700x800 pixels (resizable, minimum 600x700)
- **Position**: Centered on screen at startup
- **Style**: Modern dark theme

---

## Screen 1: Login View (Initial Screen)

### Layout Description
```
┌────────────────────────────────────────────────────────┐
│  BLADE - Local Network File Transfer            [_][□][X]│
├────────────────────────────────────────────────────────┤
│                                                        │
│                    ╔═══════════╗                      │
│                    ║           ║                      │
│                    ║   BLADE   ║                      │
│                    ║   LOGO    ║                      │
│                    ║  (150x150)║                      │
│                    ║           ║                      │
│                    ╚═══════════╝                      │
│                                                        │
│                      B L A D E                         │
│              (42px, letter-spacing: 8px)              │
│                                                        │
│            Local Network File Transfer                 │
│              (14px, letter-spacing: 2px)              │
│                                                        │
│                                                        │
│    ┌───────────────────────────────────────────┐     │
│    │                                           │     │
│    │  Username (Optional)                      │     │
│    │  ┌─────────────────────────────────────┐ │     │
│    │  │ Enter username                      │ │     │
│    │  └─────────────────────────────────────┘ │     │
│    │                                           │     │
│    │  Password (Optional)                      │     │
│    │  ┌─────────────────────────────────────┐ │     │
│    │  │ ••••••••••                          │ │     │
│    │  └─────────────────────────────────────┘ │     │
│    │                                           │     │
│    └───────────────────────────────────────────┘     │
│                                                        │
│    ┌───────────────────────────────────────────┐     │
│    │    Start with Authentication              │     │
│    │         (Blue Gradient Button)            │     │
│    └───────────────────────────────────────────┘     │
│                                                        │
│    ┌───────────────────────────────────────────┐     │
│    │    Start without Authentication           │     │
│    │      (Transparent Border Button)          │     │
│    └───────────────────────────────────────────┘     │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Color Scheme
- **Background**: Dark gradient from #1a1a2e (top) to #0f0f1e (bottom)
- **Title Text**: White (#ffffff) with high letter-spacing
- **Subtitle Text**: Gray (#a0a0a0)
- **Form Container**: Translucent white background rgba(255,255,255,0.05)
- **Input Fields**: Dark with rgba(255,255,255,0.08) background
- **Primary Button**: Blue gradient (#4a9eff to #2d7dd2)
- **Secondary Button**: Transparent with white border

### Interactive Elements
1. **Username Field**
   - Placeholder: "Enter username"
   - Optional (no red asterisk)
   - Focus: Blue border (#4a9eff)

2. **Password Field**
   - Placeholder: "Enter password"
   - Echo mode: Password (bullets)
   - Optional (no red asterisk)
   - Focus: Blue border

3. **Start with Authentication Button**
   - Enabled: Both username AND password filled
   - Warning if only one field filled
   - Hover: Lighter blue
   - Click: Darker blue

4. **Start without Authentication Button**
   - Always enabled
   - No credential validation
   - Hover: Slightly brighter
   - Click: Dimmer

---

## Screen 2: Server Running View

### Layout Description
```
┌────────────────────────────────────────────────────────┐
│  BLADE - Local Network File Transfer            [_][□][X]│
├────────────────────────────────────────────────────────┤
│                                                        │
│                  Server Running ✓                      │
│                     (Green Text)                       │
│                                                        │
│                                                        │
│              ┌─────────────────────────┐              │
│              │                         │              │
│              │   ████████  ████████   │              │
│              │   ██    ██  ██    ██   │              │
│              │   ██ QR ██  ██ QR ██   │              │
│              │   ██ CODE█  ██CODE ██   │              │
│              │   ████████  ████████   │              │
│              │   ██    ██  ██    ██   │              │
│              │   ██    ██  ██    ██   │              │
│              │   ████████  ████████   │              │
│              │                         │              │
│              │     (300x300 QR)        │              │
│              └─────────────────────────┘              │
│                 (White background,                     │
│                  20px border-radius)                   │
│                                                        │
│                                                        │
│         ┌─────────────────────────────────┐           │
│         │   http://192.168.1.10           │           │
│         │   (Blue #4a9eff, 18px mono)     │           │
│         └─────────────────────────────────┘           │
│                                                        │
│      Web Interface: Port 80 | File Transfer: Port 8080│
│                  (Gray, 14px)                          │
│                                                        │
│   Scan the QR code or visit the URL from any device   │
│                 on your network                        │
│              (Gray, 12px, centered)                    │
│                                                        │
└────────────────────────────────────────────────────────┘
```

### Color Scheme
- **Background**: Same dark gradient (#1a1a2e to #0f0f1e)
- **Status Text**: Green (#4ade80) indicating success
- **QR Container**: White (#ffffff, 95% opacity)
- **QR Code**: Black modules on white background
- **URL Box**: Blue text (#4a9eff) on blue-tinted background
- **Info Text**: Gray (#a0a0a0)

### Interactive Elements
1. **QR Code**
   - Generated from URL using qrcodegen library
   - Size: 300x300 pixels (adjustable by window size)
   - Border: 4 modules
   - Scale: 8 pixels per module
   - Non-interactive (static display)

2. **URL Text**
   - Selectable (can copy with mouse)
   - Monospace font (Consolas/Courier New)
   - Format: http://[IP_ADDRESS]
   - Example: http://192.168.1.10

3. **Port Information**
   - Displays both ports:
     - Web Interface: Port 80
     - File Transfer: Port 8080
   - Non-interactive (informational only)

---

## Visual Hierarchy

### Typography Scale
```
Level 1 (Title):        42px, Segoe UI Light, letter-spacing: 8px
Level 2 (Status):       24px, Segoe UI Light, letter-spacing: 2px
Level 3 (URL):          18px, Consolas/Courier, monospace
Level 4 (Body):         14px, Segoe UI Regular
Level 5 (Labels):       12px, Segoe UI Medium
Level 6 (Help):         12px, Segoe UI Regular
```

### Spacing System
```
XL Spacing:   50px  (Outer margins, major sections)
L Spacing:    40px  (Inner container padding)
M Spacing:    30px  (Between major elements)
S Spacing:    20px  (Between related elements)
XS Spacing:   15px  (Within elements, button padding)
```

### Border Radius Scale
```
Large:   20px  (QR container)
Medium:  15px  (Form container)
Small:   10px  (Buttons, inputs)
```

---

## State Transitions

### Startup → Login View
```
Application Launch
       ↓
Window appears (fade in)
       ↓
Logo loads
       ↓
Form elements render
       ↓
Buttons become interactive
```

### Login View → Server View
```
Button Click
       ↓
Validation (if auth mode)
       ↓
Buttons disabled
       ↓
Server initialization
       ↓
Network detection
       ↓
QR code generation
       ↓
Login view hides
       ↓
Server view shows
       ↓
Elements render sequentially:
  1. Status text
  2. QR code container
  3. QR code image
  4. URL box
  5. Port information
  6. Instructions
```

---

## Responsive Behavior

### Window Resize
- **Minimum Size**: 600x700 pixels (enforced)
- **Maximum Size**: Unrestricted
- **Behavior**: 
  - Widgets maintain aspect ratio
  - Text remains readable
  - QR code scales proportionally
  - Buttons stay full-width within containers

### Element Scaling
| Element | Behavior |
|---------|----------|
| Logo | Fixed 150x150px |
| Form Container | Max-width 400px, centered |
| QR Container | Max-width 400px, centered |
| Buttons | Full width of container |
| Text | Fixed font sizes |

---

## Accessibility Features

### Keyboard Navigation
- **Tab Order**: 
  1. Username field
  2. Password field
  3. Auth button
  4. No-auth button
- **Enter Key**: Submits form (starts server)
- **Escape Key**: (Future) Close/minimize window

### Text Readability
- **Contrast Ratios**:
  - Title on dark: 15:1 (excellent)
  - Body on dark: 10:1 (excellent)
  - Labels on dark: 7:1 (good)
- **Font Sizes**: Minimum 12px (readable)

### Visual Indicators
- **Focus States**: Blue outline on inputs
- **Hover States**: Brightness changes on buttons
- **Loading States**: Button disable with opacity
- **Error States**: Red warning messages

---

## Error and Warning Displays

### Authentication Warning
```
┌─────────────────────────────────────────┐
│  ⚠  Authentication Required             │
├─────────────────────────────────────────┤
│                                         │
│  Please enter both username and         │
│  password to start with authentication. │
│                                         │
│  Or use 'Start without Authentication'  │
│  button.                                │
│                                         │
│             [  OK  ]                    │
└─────────────────────────────────────────┘
```

### Server Start Error
```
┌─────────────────────────────────────────┐
│  ✖  Error                               │
├─────────────────────────────────────────┤
│                                         │
│  Failed to start server. Port may be    │
│  in use or requires administrator       │
│  privileges.                            │
│                                         │
│             [  OK  ]                    │
└─────────────────────────────────────────┘
```

### Network Initialization Error
```
┌─────────────────────────────────────────┐
│  ✖  Error                               │
├─────────────────────────────────────────┤
│                                         │
│  Failed to initialize network           │
│  subsystem                              │
│                                         │
│             [  OK  ]                    │
└─────────────────────────────────────────┘
```

---

## Animation and Transitions

### Subtle Animations
- **Button Hover**: 150ms ease transition
- **Input Focus**: 200ms ease border color
- **View Switch**: Instant hide/show (no animation)
- **QR Code**: Instant render (no fade)

### Performance
- **UI Updates**: 60 FPS target
- **Render Time**: < 16ms per frame
- **QR Generation**: < 100ms
- **View Switch**: < 50ms

---

## Platform-Specific Details

### Windows 10/11
- **Title Bar**: Native Windows style
- **Window Controls**: Standard minimize/maximize/close
- **Font Rendering**: ClearType antialiasing
- **DPI Scaling**: Automatic (handles 125%, 150%, 200%)

### Taskbar Integration
- **Icon**: blade.ico (16x16, 32x32, 64x64 sizes)
- **Tooltip**: "BLADE - Local Network File Transfer"
- **Right-click**: (Future) Context menu

---

## User Flow Diagrams

### Quick Start Flow (No Auth)
```
Launch App
    ↓
See Login Screen
    ↓
Click "Start without Authentication"
    ↓
Server Starts
    ↓
See QR Code
    ↓
Scan with Phone
    ↓
Transfer Files
```

### Secure Start Flow (With Auth)
```
Launch App
    ↓
See Login Screen
    ↓
Type Username
    ↓
Type Password
    ↓
Click "Start with Authentication"
    ↓
Server Starts (Auth Enabled)
    ↓
See QR Code
    ↓
Share URL + Credentials
    ↓
Users Login on Web
    ↓
Transfer Files
```

---

## Design Philosophy

### Minimalist Approach
- Clean lines
- Generous whitespace
- Limited color palette
- Clear typography

### Monochrome Focus
- Primary: Shades of blue (#4a9eff family)
- Success: Green (#4ade80)
- Error: Red (for warnings/errors)
- Neutral: Grays and whites

### Industrial Aesthetic
- Sharp edges (blade motif)
- Geometric shapes
- Technical fonts (monospace for URLs)
- Professional appearance

---

## Comparison: CLI vs GUI

### Visual Comparison
```
CLI Version:                    GUI Version:
┌─────────────────────┐        ┌─────────────────────┐
│ C:\> blade.exe      │        │   ╔═══════════╗    │
│                     │        │   ║ BLADE Logo║    │
│ BLADE Server        │        │   ╚═══════════╝    │
│ Running...          │        │      B L A D E      │
│                     │        │                     │
│ Web: http://192...  │        │  [Username Field]   │
│                     │        │  [Password Field]   │
│ QR Code:            │        │                     │
│ ██ ██ ██ ██         │        │  [Start with Auth]  │
│ (ASCII art)         │        │  [Start No Auth]    │
│                     │        │                     │
│ Ctrl+C to stop      │        │                     │
└─────────────────────┘        └─────────────────────┘
     (Console)                      (Qt Window)
```

---

## Tips for Best Visual Experience

1. **Monitor**: Use at least 1920x1080 resolution
2. **Scaling**: 100% or 125% Windows scaling works best
3. **Lighting**: Dark theme looks best in low-light environments
4. **Font**: Ensure Segoe UI is available (default on Windows 10+)
5. **Graphics**: Hardware acceleration enabled for smooth rendering

---

## Future Visual Enhancements

### Planned
- Animated status indicators
- Progress bars for file transfers
- Notification toasts
- System tray icon with menu
- Dark/light theme toggle
- Customizable accent colors

### Under Consideration
- Animated QR code generation
- Live connection counter
- Transfer speed graph
- File preview thumbnails
- Drag-and-drop zones

---

This visual guide helps users understand what to expect when using the BLADE GUI application. The interface is designed to be intuitive, professional, and easy to use while maintaining a modern, minimalist aesthetic that matches the BLADE brand.

