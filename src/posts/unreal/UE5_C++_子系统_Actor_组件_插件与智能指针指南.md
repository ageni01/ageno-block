# UE5 C++：子系统、Actor、组件、插件与智能指针

本文以 UE5 的常见游戏项目为背景，说明“功能应该放在哪里”、如何写出可用的 C++ 骨架，以及对象引用应如何选择。示例中的 `MYGAME_API` 请替换成你的模块 API 宏。

## 1. 先用一句话区分它们

| 类型 | 核心职责 | 是否在场景中有实体/Transform | 典型例子 |
|---|---|---:|---|
| `AActor` | 世界里的独立对象 | 是 | 门、敌人、可拾取物、Spawner |
| `UActorComponent` | 挂在 Actor 上的可复用能力 | 否 | 背包、属性、交互、战斗能力 |
| `USceneComponent` | 有空间变换的组件 | 是（相对 Actor） | 根节点、摄像机挂点、武器挂点 |
| `UPrimitiveComponent` | 可渲染/碰撞的场景组件 | 是 | StaticMesh、SkeletalMesh、BoxCollision |
| `USubsystem` | 某个生命周期范围内的全局服务 | 否 | 存档、匹配、任务、UI 路由 |
| 插件（Plugin） | 可独立启停、分发的功能模块集合 | 取决于内容 | 通用战斗系统、编辑器工具、SDK 接入 |

一个实用判断：**需要放进关卡、被选中和拥有位置就用 Actor；属于某个 Actor 的能力就用组件；不属于单个 Actor、需要统一管理就用 Subsystem；要跨项目复用或独立发布就做插件。**

---

## 2. Subsystem：全局服务的正确归宿

Subsystem 由 UE 自动创建和销毁，避免了手写 Singleton、`new/delete` 和生命周期混乱。不要用它承载某个关卡中“唯一存在”的实体；那类对象通常仍应是 Actor。

### 2.1 常用 Subsystem 的生命周期与获取方式

| 类 | 生命周期范围 | 适用场景 | 获取方式 |
|---|---|---|---|
| `UGameInstanceSubsystem` | 游戏启动到退出，跨关卡 | 存档、账号、全局配置、网络服务 | `GetGameInstance()->GetSubsystem<T>()` |
| `UWorldSubsystem` | 一个 `UWorld`，切图/PIE 世界会重建 | 关卡规则、世界事件、环境系统 | `World->GetSubsystem<T>()` |
| `ULocalPlayerSubsystem` | 一个本地玩家 | 输入、玩家 UI、本地设置 | `LocalPlayer->GetSubsystem<T>()` |
| `UEngineSubsystem` | 引擎进程 | 编辑器/工具级服务；慎用于游戏状态 | `GEngine->GetEngineSubsystem<T>()` |
| `UEditorSubsystem` | 编辑器进程 | 仅编辑器工具 | `GEditor->GetEditorSubsystem<T>()` |

`GameInstanceSubsystem` 在多人游戏中是“每个客户端/服务器进程各有一个”，不是所有玩家共享的服务器权威对象；权威同步状态应考虑 `GameState`、`PlayerState` 或服务端逻辑。

### 2.2 示例：跨关卡的存档子系统

`SaveServiceSubsystem.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveServiceSubsystem.generated.h"

UCLASS()
class MYGAME_API USaveServiceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Save")
    bool SavePlayerProfile(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category = "Save")
    int32 GetCoins() const { return Coins; }

private:
    int32 Coins = 0;
};
```

`SaveServiceSubsystem.cpp`

```cpp
#include "SaveServiceSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USaveServiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    // 读取存档、注册全局回调等。
}

void USaveServiceSubsystem::Deinitialize()
{
    // 解绑委托、停止异步任务等。
    Super::Deinitialize();
}

bool USaveServiceSubsystem::SavePlayerProfile(const FString& SlotName)
{
    // 实际项目中创建自己的 USaveGame 子类并序列化数据。
    return UGameplayStatics::SaveGameToSlot(nullptr, SlotName, 0);
}
```

从 Actor、组件或 Widget 使用时，应通过上下文获取：

```cpp
if (UGameInstance* GI = GetGameInstance())
{
    USaveServiceSubsystem* SaveService = GI->GetSubsystem<USaveServiceSubsystem>();
    if (SaveService)
    {
        SaveService->SavePlayerProfile(TEXT("Player_0"));
    }
}
```

> 注意：上例重点是 Subsystem 结构。`SaveGameToSlot(nullptr, ...)` 仅表示调用位置；实际必须传入 `CreateSaveGameObject` 创建的有效 `USaveGame` 实例。

### 2.3 `UWorldSubsystem` 示例与 Tick

如果服务与当前 World 强绑定，使用 `UWorldSubsystem`；需要 Tick 时继承 `UTickableWorldSubsystem`：

```cpp
UCLASS()
class MYGAME_API UWeatherSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(UWeatherSubsystem, STATGROUP_Tickables);
    }
};
```

尽量不要“为了方便”让每个 Subsystem Tick。事件、定时器和明确的更新入口通常更省性能，也更容易测试。

---

## 3. Actor 与 ActorComponent：组合优先于继承

### 3.1 Actor 的职责

Actor 是可生成、可销毁、可复制（Replication）、可放置到关卡的游戏对象。它适合负责：实体的身份、根组件、网络角色、与世界的交互，以及协调其组件。

```cpp
UCLASS()
class MYGAME_API AChestActor : public AActor
{
    GENERATED_BODY()

public:
    AChestActor();

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UInventoryComponent> Inventory;
};

AChestActor::AChestActor()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}
```

### 3.2 组件的职责

`UActorComponent` 没有 Transform，适合数据和逻辑能力；`USceneComponent` 有相对位置旋转缩放；`UPrimitiveComponent` 在此基础上还有渲染或物理/碰撞能力。

例如“背包”可同时挂给角色、箱子和商人，不必让它们继承同一个父 Actor：

```cpp
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class MYGAME_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    UFUNCTION(BlueprintCallable)
    bool AddItem(FName ItemId, int32 Count);

private:
    UPROPERTY(VisibleInstanceOnly)
    TMap<FName, int32> Items;
};

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0) return false;
    Items.FindOrAdd(ItemId) += Count;
    return true;
}
```

组件如果要网络同步：Actor 本身必须复制（`bReplicates = true`），组件调用 `SetIsReplicatedByDefault(true)`，并对要同步的字段写 `UPROPERTY(Replicated)` 与 `GetLifetimeReplicatedProps`。不要只给组件字段加 `Replicated` 就期待它自动同步。

### 3.3 生命周期与常见坑

| 时机 | 适合做什么 |
|---|---|
| 构造函数 | `CreateDefaultSubobject`、默认值；不要访问 World 或运行时对象 |
| `OnConstruction` | 根据编辑器可调参数重建预览；会反复调用，避免累积副作用 |
| `BeginPlay` | 获取其他对象、启动定时器、运行时初始化 |
| `Tick` | 每帧确有必要的逻辑；先考虑事件或 Timer |
| `EndPlay` / `Deinitialize` | 解绑委托、清理外部状态 |

不要用 `NewObject<UActorComponent>` 代替默认组件创建，除非你确实需要运行时动态组件；动态组件还要正确设置 Outer、注册（`RegisterComponent`）并保存引用。不要对 `UObject` 使用标准 C++ 的 `new` 或 `delete`。

---

## 4. 插件开发：把可复用功能装进模块

一个插件可包含一个或多个 Module。最常见结构如下：

```text
MyProject/Plugins/GameplayTools/
├── GameplayTools.uplugin
└── Source/
    └── GameplayTools/
        ├── GameplayTools.Build.cs
        ├── Public/
        │   └── GameplayToolsSubsystem.h
        └── Private/
            ├── GameplayToolsModule.cpp
            └── GameplayToolsSubsystem.cpp
```

### 4.1 最小运行时插件

`GameplayTools.uplugin`

```json
{
  "FileVersion": 3,
  "Version": 1,
  "VersionName": "1.0",
  "FriendlyName": "Gameplay Tools",
  "Category": "Gameplay",
  "CanContainContent": true,
  "Modules": [
    { "Name": "GameplayTools", "Type": "Runtime", "LoadingPhase": "Default" }
  ]
}
```

`GameplayTools.Build.cs`

```csharp
using UnrealBuildTool;

public class GameplayTools : ModuleRules
{
    public GameplayTools(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
    }
}
```

`GameplayToolsModule.cpp`

```cpp
#include "Modules/ModuleManager.h"

class FGameplayToolsModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FGameplayToolsModule, GameplayTools)
```

引擎中可从 **Edit → Plugins → New Plugin → Blank** 创建同等骨架。创建后重启编辑器或重新生成项目文件并编译。

### 4.2 Runtime 与 Editor 模块

| Module Type | 能进入打包游戏 | 用途 |
|---|---:|---|
| `Runtime` | 是 | 游戏逻辑、运行时资产、Subsystem |
| `Editor` | 否 | 菜单、资产工厂、细节面板定制、批处理工具 |
| `Developer` | 通常不进 Shipping | 开发期工具 |

编辑器代码必须放在 `Editor` 模块，且 Runtime 模块不能依赖 `UnrealEd`。常见做法是一个插件两个模块：`GameplayTools`（Runtime）+ `GameplayToolsEditor`（Editor），后者依赖前者。

### 4.3 插件依赖与 API 宏

- 在 `Build.cs` 中声明 C++ 模块依赖：头文件暴露的类型放 `PublicDependencyModuleNames`，仅 `.cpp` 使用的放 `PrivateDependencyModuleNames`。
- 需要调用另一个插件，既要在 `Build.cs` 加模块依赖，也要在 `.uplugin` 的 `Plugins` 字段声明插件依赖。
- 公共类使用模块导出的 `GAMEPLAYTOOLS_API`，不要误用主游戏模块的 `MYGAME_API`。
- 插件不要直接硬编码项目内容目录；内容资产使用 `SoftObjectPath`、`AssetManager` 或可配置项。

---

## 5. UE 智能指针与 UObject 引用：先分两大阵营

最重要的规则是：**`TSharedPtr/TUniquePtr` 管理普通 C++ 对象；`TObjectPtr/TWeakObjectPtr/TSoftObjectPtr` 引用由 GC 管理的 `UObject`。不要把 UObject 放进 `TSharedPtr` 或 `TUniquePtr`。**

| 类型 | 面向对象 | 是否拥有对象 | 常用场景 |
|---|---|---:|---|
| `TObjectPtr<T>` | `UObject` | GC 跟踪的强引用 | `UPROPERTY` 成员；UE5 默认首选 |
| `TWeakObjectPtr<T>` | `UObject` | 否 | 缓存其他 Actor，随时允许失效 |
| `TSoftObjectPtr<T>` | 资产/`UObject` | 否，不阻止卸载 | 延迟加载资产或跨关卡引用 |
| `TSoftClassPtr<T>` | `UClass` | 否 | 延迟加载角色/Widget 类 |
| `TSharedPtr<T>` | 普通 C++ 类型 | 共享所有权 | 多处共同拥有的非 UObject 数据 |
| `TSharedRef<T>` | 普通 C++ 类型 | 共享所有权且永不为空 | Slate、参数要求有效对象时 |
| `TWeakPtr<T>` | 普通 C++ 类型 | 否 | 避免 `TSharedPtr` 循环引用 |
| `TUniquePtr<T>` | 普通 C++ 类型 | 独占所有权 | 仅一个拥有者的实现细节 |

### 5.1 UObject 指针示例

```cpp
UCLASS()
class MYGAME_API ATrackerActor : public AActor
{
    GENERATED_BODY()

    // 强引用：GC 知道它仍被使用；编辑器可配置。
    UPROPERTY(EditAnywhere)
    TObjectPtr<AActor> Target;

    // 不拥有 Target；目标销毁后 IsValid() 变为 false。
    TWeakObjectPtr<AActor> LastInteractedActor;

    // 只记录资源路径，不会立即把资源加载进内存。
    UPROPERTY(EditDefaultsOnly)
    TSoftClassPtr<UUserWidget> HintWidgetClass;
};
```

使用弱引用前检查：`if (LastInteractedActor.IsValid()) { ... }`。加载软引用时用同步加载（加载界面等可接受卡顿的少量场景）或 `UAssetManager` / `FStreamableManager` 的异步加载，切勿假定 `.Get()` 一定非空。

### 5.2 普通 C++ 对象示例

```cpp
class FMatchSnapshot
{
public:
    int32 Round = 0;
};

TSharedPtr<FMatchSnapshot> Snapshot = MakeShared<FMatchSnapshot>();
TWeakPtr<FMatchSnapshot> Observer = Snapshot;

if (TSharedPtr<FMatchSnapshot> Pinned = Observer.Pin())
{
    ++Pinned->Round;
}

TUniquePtr<FMatchSnapshot> Exclusive = MakeUnique<FMatchSnapshot>();
```

`TSharedPtr` 循环引用会泄漏：A 持有 B 的 `TSharedPtr`，B 应改持有 A 的 `TWeakPtr`。`TSharedRef` 不可为空，因此从可能为空的 `TSharedPtr` 转为 `ToSharedRef()` 前必须确保有效。

### 5.3 什么时候仍使用裸指针？

函数参数或局部变量可以使用 `UObject*`，它表达“临时借用”，并且在 UE C++ 中完全正常；长期保存为成员时，优先使用带 `UPROPERTY()` 的 `TObjectPtr`。不要把裸 UObject 成员当成 GC 保活引用。UE5 中反射属性常写为 `TObjectPtr`，但 API 参数、返回值和局部值使用 `UObject*` 依然很常见。

---

## 6. 一个推荐的落地结构

以“任务系统”为例：

```text
QuestPlugin (Runtime)
├── UQuestSubsystem          # 跨关卡：任务定义、存档读写、统一事件入口
├── UQuestLogComponent       # 每个玩家各自的进行中任务、复制数据
├── AQuestTriggerActor       # 放在关卡中，负责触发范围和交互
└── UQuestDataAsset           # 任务静态配置，使用软引用指向奖励资产
```

这比把全部逻辑塞进 `GameInstance`、`Character` 或一个巨大 Actor 更清晰：世界实体由 Actor 表达，玩家私有状态属于组件，跨关卡服务放 Subsystem，可复用边界则由插件提供。

## 7. 开发检查清单

- 能否跨关卡？能：优先评估 `UGameInstanceSubsystem`。
- 是否一世界一份且切图重置？是：评估 `UWorldSubsystem`。
- 是否有位置、碰撞、关卡摆放或网络实体身份？是：`AActor`。
- 是否应被多个 Actor 复用？是：`UActorComponent`。
- 是否是可拆分、可复用或可分发的能力？是：Runtime 插件；编辑器辅助另建 Editor 模块。
- 是否是 `UObject`？是：交给 GC，使用 `TObjectPtr`/弱引用/软引用，不能 `MakeShared`。
- 是否只是普通 C++ 数据？是：根据所有权选 `TUniquePtr` 或 `TSharedPtr`。

