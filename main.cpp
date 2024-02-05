/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
#include "version.h"
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSettings>

int main(int argc, char *argv[]) {
  QGuiApplication app(argc, argv);

  app.setApplicationName(APPLICATION_NAME);
  app.setApplicationVersion(APPLICATION_VERSION);
  app.setOrganizationName(ORGANIZATION_NAME);
  app.setOrganizationDomain(ORGANIZATION_DOMAIN);

  // 自動起動の確認
  QCommandLineParser parser;
  QCommandLineOption lopt("l", "launched by nizima LIVE");
  QCommandLineOption popt("p", "port number", "port");
  parser.addOptions({lopt, popt});
  parser.process(app);
  bool live = parser.isSet(lopt) && parser.isSet(popt);
  int port = parser.value(popt).toInt();

  QQmlApplicationEngine engine;
  QObject::connect(
          &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
          []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  QObject::connect(
          &engine, &QQmlApplicationEngine::objectCreated,
          &app, [=](QObject *object, const QUrl &url) {
            if (url.fileName() != "Main.qml") return;
            if (live) QMetaObject::invokeMethod(object, "autoConnect", QVariant(port));
          },
          Qt::SingleShotConnection);
  engine.loadFromModule("NLPluginApi", "Main");

  return app.exec();
}
