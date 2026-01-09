#pragma once

#include <QApplication>
#include <memory>

namespace blade {

class MainWindow;

class Application {
public:
    Application(int argc, char* argv[]);
    ~Application();

    int run();

private:
    std::unique_ptr<QApplication> qapp_;
    std::unique_ptr<MainWindow> mainWindow_;
};

} // namespace blade

