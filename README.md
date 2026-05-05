# UE5 Move Debug Tool

目前提供了 2 套简单的 UE5 移动 Debug 工具。

## 1. CMC 内置移动调试

在 `CharacterMovementComponent` 中输入指令 `a.CharacterMovement.MovementDebugInfo` 即可开启。

- 显示玩家的位置、速度等信息
- 可同步显示服务器、模拟端的数据
- 支持按需扩展更多调试信息

## 2. Visual Logger 驱动的移动调试

第二套方案依赖 `VisualLogger`，源码位于 `UMovementInfoSubsystem` 中，需要在其他触发移动的地方手动埋桩。

- 示例代码可参考 `UHYCharacterMovementComponent::TickComponent`
- 使用 RAII 记录生命周期内的移动变化
- 只要发生变化就会打印信息
- 目前桩的基础功能已经完成，但仍有不足和拓展点，已在 `UMovementInfoSubsystem` 的注释中补充

## 文件说明

- `Source/DebugMove/MovementInfoSubsystem.h`
- `Source/DebugMove/MovementInfoSubsystem.cpp`
- `Source/DebugMove/HYCharacterMovementComponent.h`
- `Source/DebugMove/HYCharacterMovementComponent.cpp`
