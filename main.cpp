#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include "peer.h"

#include <spdlog/spdlog.h>
#include <iostream>

#define DEFAULT_USER "Retard Moronovich: "

int main(int argc, char *argv[])
{
    Peer peer(5555);
    peer.connect("127.0.0.1", 5555);
    return 0;


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
