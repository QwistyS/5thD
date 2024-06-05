#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include "peer.h"



#define DEFAULT_USER "Retard Moronovich: "

int main(int argc, char *argv[])
{
    Peer peer;
    peer.connect("127.0.0.1", 5555);
    // Additional logic as needed
    
    QApplication a(argc, argv);

    // Main window
    QWidget mainWindow;
    QVBoxLayout layout(&mainWindow);
    mainWindow.setWindowTitle("5thD Chat");

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
            chatWindow.append(DEFAULT_USER + message);
            inputBox.clear();
        }
    });

    mainWindow.show();

    return a.exec();
}
