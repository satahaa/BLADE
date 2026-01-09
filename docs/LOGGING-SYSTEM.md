# BLADE Logging System Setup

## Overview
The BLADE application now uses a file-based logging system instead of console output. All debug messages, connection logs, and error messages are written to timestamped log files.

## Directory Structure

### Created Directories:
```
BLADE/
├── logs/                           # Project root logs (for development)
│   ├── .gitignore                  # Git ignore rules for log files
│   ├── .gitkeep                    # Keeps directory in git
│   ├── README.md                   # Documentation
│   └── blade_sample.log            # Sample log file format
├── bin/logs/                       # Production logs (when running from bin/)
└── cmake-build-debug/logs/         # Debug build logs
```

## Log Files

### Naming Convention
- Format: `blade_YYYYMMDD_HHMMSS.log`
- Example: `blade_20260109_031530.log`
- A new log file is created for each server session

### Log Entry Format
```
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] Message
```

### Log Levels
- **DEBUG**: Detailed debugging information (constructor calls, configuration)
- **INFO**: General informational messages (server start, connections)
- **WARNING**: Warning messages for potential issues
- **ERROR**: Error messages for failures

## Implementation Files

### Logger Class
- **Header**: `include/Logger.h`
- **Implementation**: `src/Logger.cpp`
- **Pattern**: Singleton pattern for global access
- **Thread Safety**: Uses mutex for concurrent logging

### Key Features
1. **Automatic Directory Creation**: Creates `logs/` folder if it doesn't exist
2. **Timestamped Entries**: Each log entry includes millisecond-precision timestamp
3. **Thread-Safe**: Supports concurrent logging from multiple threads
4. **Automatic Flushing**: Ensures logs are written immediately
5. **Session Tracking**: Logs session start and end markers

## Usage in Code

```cpp
#include "Logger.h"

// Get singleton instance and log
Logger::getInstance().info("Server started");
Logger::getInstance().debug("Debug information");
Logger::getInstance().warning("Warning message");
Logger::getInstance().error("Error occurred");
```

## Modified Files

All console output (`std::cout`, `std::cerr`) has been replaced with Logger calls in:
- `src/Server.cpp`
- `src/HTTPServer.cpp`

## Git Configuration

The `.gitignore` in `logs/` directory ensures:
- Log files (`*.log`) are NOT tracked by git
- Documentation and structure files ARE tracked
- Sample log file is tracked for reference

## Testing

To verify logging works:
1. Run BLADE application
2. Click "Start with Authentication" or "Start without Authentication"
3. Check `bin/logs/` directory for new log file
4. Log file will contain all server initialization and operation messages

## Example Log Output

```
[2026-01-09 03:15:30.123] [INFO] === BLADE Log Session Started ===
[2026-01-09 03:15:30.127] [INFO] HTTP Server initialized on port 80
[2026-01-09 03:15:30.151] [INFO]    BLADE Server Started Successfully
[2026-01-09 03:15:30.153] [INFO] Web Interface Access: http://192.168.1.100
[2026-01-09 03:15:35.234] [INFO] [HTTP CLIENT] 192.168.1.101 connected
```

## Maintenance

- Log files accumulate over time
- Manually delete old log files to free disk space
- Each session creates a new file (not append to existing)
- No automatic log rotation implemented (manual cleanup required)

## Benefits

✅ **Silent Operation**: No console spam, clean user experience
✅ **Debugging**: Full history of server operations for troubleshooting
✅ **Audit Trail**: Track all connections and activities
✅ **Production Ready**: Professional logging suitable for deployment
✅ **Thread-Safe**: Safe for multi-threaded server operations

