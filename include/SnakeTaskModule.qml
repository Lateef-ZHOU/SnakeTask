pragma ComponentBehavior: Bound
import QtQuick
import Lidar.TaskUI
import QtQuick.Controls

// 两套层级在末端连接：本页面独持具体任务，计划与协程分别保持本编排的实现。
TaskModule {
    id: root
    objectName: "SnakeTaskModule"
    readonly property string name: internals.task.displayName
    readonly property string subtitle: internals.task.subtitle
    readonly property bool running: internals.task.running
    settingsParameters: internals.task.settingsParameters
    planPage: SnakeTaskPlan { task: internals.task }
    mainPages: [Page {
        title: qsTr("任务预览")
        contentItem: TaskPlanView {
            task: internals.task
            settings: root
            plan: root.planPage
            pathStyle: BeamCompass.Snake
        }
    }]

    function start(azimuths, elevation): bool {
        return internals.task.start(gimbal, pickedCards, azimuths, elevation, accum, settle, save, saveDir,
                                    {panSpeed: root.panSpeed, tiltSpeed: root.tiltSpeed})
    }
    function stop(): void { internals.task.stop() }
    Component.onCompleted: root.registerDevices(internals.task)
    QtObject {
        id: internals
        readonly property SnakeTask task: root.SnakeTask
    }
    Connections {
        target: internals.task
        function onLogMessage(text): void { root.logMessage(text) }
        function onDeviceRemoved(device): void { root.forgetDevice(device) }
    }
}
