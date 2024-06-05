#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Main window
    QWidget mainWindow;
    QVBoxLayout layout(&mainWindow);
    mainWindow.setWindowTitle("Telegram Replica");

    // Chat window
    QTextEdit chatWindow;
    chatWindow.setReadOnly(true);
    layout.addWidget(&chatWindow);

    // Input box
    QLineEdit inputBox;
    layout.addWidget(&inputBox);

    // Send button
    QPushButton sendButton("Send");
    layout.addWidget(&sendButton);

    // Connect send button click signal to send message
    QObject::connect(&sendButton, &QPushButton::clicked, [&](){
        QString message = inputBox.text();
        if (!message.isEmpty()) {
            chatWindow.append("User 1: " + message);
            inputBox.clear();
        }
    });

    mainWindow.show();

    return a.exec();
}
