/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
#pragma once
#include "message.h"
#include <QCoreApplication>
#include <QCursor>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QWebSocket>

namespace Live2D::Nizima::PluginApi {
namespace {

QString icon64() {
  QByteArray icon{};
  QFile file(":/resources/icon.png");
  if (file.open(QIODevice::ReadOnly))
    icon = file.readAll();
  return icon.toBase64();
}

}// namespace

class Promise : public QObject {
  Q_OBJECT
  QML_ELEMENT
public:
  using QObject::QObject;

signals:
  void accepted(const QVariantMap &data, const QDateTime &);
  void rejected(const QVariantMap &data, const QDateTime &);
};

class NLPluginApi : public QObject {
  Q_OBJECT
  QML_ELEMENT
  Q_PROPERTY(bool enableAutoLaunch MEMBER m_enableAutoLaunch NOTIFY enableAutoLaunchChanged)
  Q_PROPERTY(bool autoLaunched MEMBER m_autoLaunched NOTIFY autoLaunchedChanged)
  Q_PROPERTY(int port MEMBER m_port NOTIFY portChanged)
  Q_PROPERTY(State state READ state NOTIFY stateChanged)
  Q_PROPERTY(QString pluginName MEMBER m_pluginName NOTIFY pluginNameChanged)
  Q_PROPERTY(QString pluginDeveloper MEMBER m_pluginDeveloper NOTIFY pluginDeveloperChanged)
  Q_PROPERTY(QString pluginVersion MEMBER m_pluginVersion NOTIFY pluginVersionChanged)
  Q_PROPERTY(QString token MEMBER m_token NOTIFY tokenChanged)

public:
  enum State {
    Closed,
    Connecting,
    Open,
    Established,
    Available
  };
  Q_ENUM(State)

  explicit NLPluginApi(QObject *parent = nullptr)
      : QObject(parent), m_socket(new QWebSocket) {
    m_socket->setParent(this);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &NLPluginApi::processMessage);
    connect(this, &NLPluginApi::responseReceived, this, &NLPluginApi::acceptPromise);
    connect(this, &NLPluginApi::errorReceived, this, &NLPluginApi::rejectPromise);
    connect(this, &NLPluginApi::eventReceived, this, [this](Method::Enum method, const QJsonObject &data) {
      if (method == Method::NotifyEnabledChanged) {
        if (data.value("Enabled").toBool()) {
          setState(Available);
        } else {
          setState(Established);
        }
      }
    });

    connect(m_socket, &QWebSocket::connected, this, [this] {
      setState(Open);
      establishConnection();
    });
    connect(m_socket, &QWebSocket::disconnected, this, [this] {
      setState(Closed);
    });
    connect(m_socket, &QWebSocket::errorOccurred, this, [this](QAbstractSocket::SocketError e) {
      emit socketErrorOccurred(QMetaEnum::fromType<QAbstractSocket::SocketError>().valueToKey(e), m_socket->errorString());
    });
  }

  State state() const {
    return m_state;
  }

  Q_INVOKABLE void open() {
    if (m_state != Closed) return;
    if (m_pluginName.isEmpty()) {
      qCritical() << "pluginName must be set!!";
      return;
    }
    m_socket->open(QString("ws://localhost:%1").arg(m_port));
    setState(Connecting);
  }

  Q_INVOKABLE void close() {
    if (m_state == Closed) return;
    m_socket->close();
  }

  Q_INVOKABLE Promise *send(Method::Enum method, const QJsonObject &data = {}, const QDateTime &dateTime = QDateTime::currentDateTime()) {
    static int requestId = 0;
    auto id = QString("auto_%1").arg(++requestId);

    Message request{.timestamp = dateTime,
                    .id = id,
                    .type = Type::Request,
                    .method = method,
                    .data = data};
    QJsonDocument json(request.toJson());
    m_socket->sendTextMessage(json.toJson(QJsonDocument::Compact));

    qDebug() << ">>>>>>>>>>>>>>>>   Send   >>>>>>>>>>>>>>>>";
    qDebug().noquote() << json.toJson();

    auto promise = new Promise(this);
    m_promises[id] = promise;
    return promise;
  }

signals:
  void enableAutoLaunchChanged(bool);
  void autoLaunchedChanged(bool);
  void portChanged(int);
  void stateChanged(State);
  void pluginNameChanged(const QString &);
  void pluginDeveloperChanged(const QString &);
  void pluginVersionChanged(const QString &);
  void tokenChanged(const QString &);
  void eventReceived(Method::Enum method, const QJsonObject &data, const QDateTime &);
  void responseReceived(const QString &requestId, Method::Enum method, const QJsonObject &data, const QDateTime &);
  void errorReceived(const QString &requestId, Method::Enum method, const QJsonObject &data, const QDateTime &);
  void socketErrorOccurred(const QString &errorEnumName, const QString &errorString);

private:
  bool m_enableAutoLaunch = false;
  bool m_autoLaunched = false;
  int m_port = 22022;
  State m_state = Closed;
  QString m_pluginName = "";
  QString m_pluginDeveloper = "";
  QString m_pluginVersion = "0.0.0";
  QString m_token = "";
  QPointer<QWebSocket> m_socket;
  QHash<QString, QPointer<Promise>> m_promises;

  void setState(State state) {
    if (m_state == state) return;
    if (state == Available) {
      registerPluginPath();
    }
    emit stateChanged(m_state = state);
  }

  void processMessage(const QString &text) {
    auto json = QJsonDocument::fromJson(text.toUtf8());
    auto message = Message::fromJson(json.object());
    if (message.version.isEmpty()) return;

    qDebug() << "<<<<<<<<<<<<<<<< Received <<<<<<<<<<<<<<<<";
    qDebug().noquote() << json.toJson();

    switch (message.type) {
      case Type::Event:
        emit eventReceived(message.method, message.data, message.timestamp);
        break;
      case Type::Response:
        emit responseReceived(message.id, message.method, message.data, message.timestamp);
        break;
      case Type::Error:
        emit errorReceived(message.id, message.method, message.data, message.timestamp);
        break;
      default:
        qWarning() << "Unknown Message Type Received.";
    }
  }

  void acceptPromise(const QString &requestId, Method::Enum, const QJsonObject &data, const QDateTime &dateTime) {
    if (auto promise = m_promises.take(requestId)) {
      emit promise->accepted(data.toVariantMap(), dateTime);
      promise->setParent(nullptr);// Allow GC to delete Promise
    }
  }

  void rejectPromise(const QString &requestId, Method::Enum, const QJsonObject &data, const QDateTime &dateTime) {
    if (auto promise = m_promises.take(requestId)) {
      emit promise->rejected(data.toVariantMap(), dateTime);
      promise->setParent(nullptr);// Allow GC to delete Promise
    }
  }

  void registerPlugin() {
    if (m_state != Open) return;
    if (m_pluginName.isEmpty()) {
      qCritical() << "pluginName must be set!!";
      close();
      return;
    }
    QJsonObject data{{"Name", m_pluginName},
                     {"Developer", m_pluginDeveloper},
                     {"Version", m_pluginVersion},
                     {"Icon", icon64()}};
    auto promise = send(Method::RegisterPlugin, data);
    connect(promise, &Promise::accepted, this, [this, promise](const QVariantMap &data) {
      m_token = data.value("Token").toString();
      emit tokenChanged(m_token);
      setState(Established);
      promise->deleteLater();
    });
    connect(promise, &Promise::rejected, this, [this, promise] {
      qWarning() << "RegisterPlugin rejected.";
      close();
      promise->deleteLater();
    });
  }

  void establishConnection() {
    if (m_state != Open) return;
    if (m_pluginName.isEmpty()) {
      qCritical() << "pluginName must be set!!";
      close();
      return;
    }
    if (m_token.isEmpty()) {
      registerPlugin();
    } else {
      auto promise = send(Method::EstablishConnection, {{"Name", m_pluginName},
                                                        {"Token", m_token},
                                                        {"Version", m_pluginVersion}});
      connect(promise, &Promise::accepted, this, [this, promise](const QVariantMap &data) {
        setState(Established);
        if (data.value("Enabled").toBool())
          setState(Available);
        promise->deleteLater();
      });
      connect(promise, &Promise::rejected, this, [this, promise] {
        registerPlugin();
        promise->deleteLater();
      });
    }
  }

  void registerPluginPath() {
    if (m_enableAutoLaunch) {
      auto promise = send(Method::RegisterPluginPath, {{"PluginPath", QCoreApplication::applicationFilePath()}});
      connect(promise, &Promise::accepted, promise, &QObject::deleteLater);
      connect(promise, &Promise::rejected, promise, &QObject::deleteLater);
    }
  }
};

class Cursor : public QObject {
  Q_OBJECT
  QML_SINGLETON
  QML_ELEMENT
public:
  using QObject::QObject;
  Q_INVOKABLE static QPoint pos() {
    return QCursor::pos();
  }
};

}// namespace Live2D::Nizima::PluginApi
