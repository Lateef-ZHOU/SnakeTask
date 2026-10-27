#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQmlListReference>
#include <QtQml/QQmlExtensionPlugin>
#include <QDebug>
#include <memory>
#include <QPointer>
#include <QJSValue>
#include "Device/Base/DeviceBase.h"
Q_IMPORT_QML_PLUGIN(LidarUIPlugin)
Q_IMPORT_QML_PLUGIN(LidarChartsPlugin)
Q_IMPORT_QML_PLUGIN(LidarTaskUIPlugin)
Q_IMPORT_QML_PLUGIN(LidarSnakeTaskPlugin)
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    QQmlEngine engine;
    bool warned = false;
    QObject::connect(&engine, &QQmlEngine::warnings, &engine, [&](const auto &) { warned = true; });
    for (const bool simOnly : {false, true}) {
        QQmlComponent component(&engine);
        component.setData(simOnly ? "import Lidar.SnakeTask\nSnakeTaskSimOnlyModule { gimbal: null; card: null }" : "import Lidar.SnakeTask\nSnakeTaskModule { gimbals: []; cards: [] }", QUrl());
        const QVariantMap parameters{{"azimuths", QVariantList{17.0, 107.0, 197.0, 287.0}}, {"elevation", 53.0},
                                     {"accum", 7}, {"settle", 4}};
        std::unique_ptr<QObject> page(simOnly ? component.createWithInitialProperties(parameters) : component.create());
        if (!page) { qCritical() << component.errors(); return 1; }
        const QQmlListReference charts(page.get(), "mainPages");
        const QQmlListReference sections(page.get(), "sections");
        if (charts.count() < 1 || sections.count() < 1) return 2;
        if (page->property("name").toString().isEmpty()) return 3;
        if (simOnly) {
            if (sections.count() != 2) return 7;
            if (page->metaObject()->indexOfProperty("save") >= 0 ||
                page->metaObject()->indexOfProperty("saveDir") >= 0 ||
                page->metaObject()->indexOfProperty("pickedCards") >= 0) return 8;
            auto *plan = page->property("planPage").value<QObject *>();
            const auto valueList = [](QVariant value) {
                return value.metaType() == QMetaType::fromType<QJSValue>()
                    ? value.value<QJSValue>().toVariant().toList() : value.toList();
            };
            if (!plan || plan->property("elevation").toDouble() != 53.0 ||
                valueList(plan->property("azimuths")) != parameters.value("azimuths").toList() ||
                page->property("accum").toInt() != 7 || page->property("settle").toInt() != 4) return 9;
            // 编辑页面计划会立即反映到官邸；没有另一份角度缓存。
            plan->setProperty("elevation", 48.0);
            if (page->property("elevation").toDouble() != 48.0) return 10;
            page->setProperty("elevation", 55.0);
            if (plan->property("elevation").toDouble() != 55.0) return 11;
            const int start = page->metaObject()->indexOfMethod("start()");
            const int stop = page->metaObject()->indexOfMethod("stop()");
            if (start < 0 || stop < 0 || page->metaObject()->indexOfMethod("start(QVariant,QVariant)") >= 0) return 12;
            // 空设备只会拒绝启动；用于验证无参命令能从 Qt 元对象调用。
            bool started = true;
            if (!QMetaObject::invokeMethod(page.get(), "start", Q_RETURN_ARG(bool, started)) || started ||
                !QMetaObject::invokeMethod(page.get(), "stop")) return 13;
        }

        QObject *backend = nullptr;
        int backends = 0;
        for (auto *child : page->children()) {
            if (QByteArray(child->metaObject()->className()) == "SnakeTask") {
                backend = child;
                ++backends;
            }
        }
        if (backends != 1 || backend->parent() != page.get()) return 5;
        const QPointer<QObject> lifetime(backend);
        app.processEvents();
        page.reset();
        if (lifetime) return 6;
    }
    // 每个创建参数缺席都必须失败，不能退回计划页内部的默认值。
    for (const auto *missing : {"azimuths", "elevation", "accum", "settle"}) {
        QQmlComponent component(&engine);
        component.setData("import Lidar.SnakeTask\nSnakeTaskSimOnlyModule { gimbal: null; card: null }", QUrl());
        QVariantMap parameters{{"azimuths", QVariantList{17.0, 107.0, 197.0, 287.0}}, {"elevation", 53.0},
                               {"accum", 7}, {"settle", 4}};
        parameters.remove(QLatin1String(missing));
        std::unique_ptr<QObject> page(component.createWithInitialProperties(parameters));
        if (page || component.errors().empty()) return 14;
        bool namesMissing = false;
        for (const auto &error : component.errors())
            namesMissing |= error.description().contains(QLatin1String(missing));
        if (!namesMissing) return 15;
    }
    return warned ? 4 : 0;
}
