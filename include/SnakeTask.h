#pragma once
#include <vector>
#include <QtQml/qqmlregistration.h>
#include <QScopeGuard>
#include "TaskBase.h"
#include "WindProfile.h"

// 蛇形四轴：页面持世界角输入，协程生成 Collector 并直接表达编排。
class SnakeTask final : public TaskBase
{
    Q_OBJECT
    QML_ELEMENT
    QML_ATTACHED(SnakeTask)
    QML_UNCREATABLE("由对应 QML 官邸创建和拥有")
public:
    static SnakeTask *qmlAttachedProperties(QObject *owner) { return new SnakeTask(owner); }
    explicit SnakeTask(QObject *parent) : TaskBase(parent) {}
    ~SnakeTask() override { cancelRun(); }
    [[nodiscard]] QString displayName() const override { return QStringLiteral("蛇形四轴"); }
    [[nodiscard]] QString subtitle() const override { return QStringLiteral("往返循环"); }
    [[nodiscard]] int beamCount() const noexcept override { return WindProfile::kBeams; }
private:
    DeviceCoro run(LockRequest, GMBFamily &gmb, Cards cards, QList<double> worldAzimuths,
                   double worldElevation, int accumSec, std::chrono::milliseconds hold,
                   std::optional<GMBMoveParameters::Speeds> speeds, std::optional<QDir> saveRoot) override
    {
        const auto completion = qScopeGuard([this] {
            QMetaObject::invokeMethod(this, &SnakeTask::refreshRunState, Qt::QueuedConnection);
        });
        const auto lock = co_await HeldLock{};
        if (!lock)
        {
            Q_EMIT logMessage("设备被占用或未连接，任务未启动");
            co_return;
        }
        // 全部世界目标先经官邸 paramsFor 校验，再发任何任务指令；协程只保存运动参数，到位结果仍是 Collector。
        std::vector<GMBMoveParameters> targets; // 参数字段只读、不可赋值：用只需移动构造的 vector
        for (const double azimuth : worldAzimuths)
        {
            auto parameters = gmb.paramsFor(gmb.gimbalOf(WorldModel(azimuth, worldElevation)), hold, speeds);
            if (!parameters)
            {
                Q_EMIT logMessage("转台目标或运动参数无效，任务未启动");
                co_return;
            }
            targets.push_back(std::move(*parameters));
        }
        QDateTime sessionTime = beginSession();
        {
            const auto moved = co_await moveTo(lock, gmb, targets[0], 0);
            if (moved.empty())
            {
                Q_EMIT logMessage("转台 失败 / 设备断开 / 任务停止");
                co_return;
            }
            const auto framed = co_await acquireAt(lock, gmb, cards, moved, 0, accumSec, saveRoot, sessionTime, false);
            if (framed.empty())
            {
                Q_EMIT logMessage("采集卡 失败 / 设备断开 / 任务停止");
                co_return;
            }
            completeBeam();
        }
        constexpr int last = WindProfile::kBeams - 1;
        for (;;)
        {
            for (int beam = 1; beam <= last; ++beam)
            {
                const auto moved = co_await moveTo(lock, gmb, targets[beam], beam);
                if (moved.empty())
                {
                    Q_EMIT logMessage("转台 失败 / 设备断开 / 任务停止");
                    co_return;
                }
                const auto framed = co_await acquireAt(lock, gmb, cards, moved, beam, accumSec, saveRoot, sessionTime, beam == last);
                if (framed.empty())
                {
                    Q_EMIT logMessage("采集卡 失败 / 设备断开 / 任务停止");
                    co_return;
                }
                completeBeam();
            }
            sessionTime = QDateTime::currentDateTime();

            for (int beam = last - 1; beam >= 0; --beam)
            {
                const auto moved = co_await moveTo(lock, gmb, targets[beam], beam);
                if (moved.empty())
                {
                    Q_EMIT logMessage("转台 失败 / 设备断开 / 任务停止");
                    co_return;
                }
                const auto framed = co_await acquireAt(lock, gmb, cards, moved, beam, accumSec, saveRoot, sessionTime, beam == 0);
                if (framed.empty())
                {
                    Q_EMIT logMessage("采集卡 失败 / 设备断开 / 任务停止");
                    co_return;
                }
                completeBeam();
            }
            sessionTime = QDateTime::currentDateTime();
        }
    }
};
