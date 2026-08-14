# Unreal Engine C++ — Subsystem、多线程与异步详解

> 本文档基于 UE5，示例模块名为 `MyGame`，请替换为实际项目模块名。

---

## 一、Subsystem 子系统

### 1.1 什么是 Subsystem

Subsystem 是 UE4.20+ 引入的一套**生命周期自动管理**的框架。每种 Subsystem 都由引擎自动创建与销毁，开发者无需手动 `NewObject` 或管理引用，天然解决了"全局管理器"的单例问题。

引擎提供 5 种内置 Subsystem 基类，对应不同作用域：

| 基类 | 生命周期 | 典型用途 |
| --- | --- | --- |
| `UGameInstanceSubsystem` | 与 GameInstance 同寿（跨关卡持久） | 全局存档、成就、数据服务 |
| `UWorldSubsystem` | 与 UWorld 同寿（单关卡） | 关卡级管理器、天气、导航 |
| `ULocalPlayerSubsystem` | 与 LocalPlayer 同寿（本地玩家） | 输入设置、UI 偏好 |
| `UEditorSubsystem` | 编辑器进程寿命 | 编辑器工具、自动化 |
| `UTickableWorldSubsystem` | 与 UWorld 同寿 + 自带 Tick | 需要每帧更新的关卡系统 |

### 1.2 自定义 Subsystem 步骤

#### 步骤 1：声明头文件

```cpp
// MyGameInstanceSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MyGameInstanceSubsystem.generated.h"

UCLASS()
class MYGAME_API UMyGameInstanceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // 生命周期回调（可选）
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // 业务接口
    UFUNCTION(BlueprintCallable, Category = "MyGame|Save")
    void SaveGame(const FString& SlotName);

    UFUNCTION(BlueprintCallable, Category = "MyGame|Save")
    bool LoadGame(const FString& SlotName);

    UFUNCTION(BlueprintPure, Category = "MyGame|Save")
    bool HasSaveData(const FString& SlotName) const;

private:
    UPROPERTY()
    TObjectPtr<class UMySaveGame> CurrentSaveGame;
};
```

#### 步骤 2：实现

```cpp
// MyGameInstanceSubsystem.cpp
#include "MyGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "MySaveGame.h"

void UMyGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("MyGameInstanceSubsystem Initialized"));
}

void UMyGameInstanceSubsystem::Deinitialize()
{
    CurrentSaveGame = nullptr;
    Super::Deinitialize();
}

void UMyGameInstanceSubsystem::SaveGame(const FString& SlotName)
{
    if (!CurrentSaveGame)
    {
        CurrentSaveGame = NewObject<UMySaveGame>();
    }

    UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SlotName, 0);
}

bool UMyGameInstanceSubsystem::LoadGame(const FString& SlotName)
{
    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, 0);
    CurrentSaveGame = Cast<UMySaveGame>(Loaded);
    return CurrentSaveGame != nullptr;
}

bool UMyGameInstanceSubsystem::HasSaveData(const FString& SlotName) const
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}
```

### 1.3 获取 Subsystem

引擎自动管理 Subsystem 生命周期，通过以下方式获取：

```cpp
// 方式 1：从 World 获取
UWorldSubsystem* WorldSub = GetWorld()->GetSubsystem<UWorldSubsystem>();

// 方式 2：从 GameInstance 获取
UMyGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<UMyGameInstanceSubsystem>();

// 方式 3：从 LocalPlayer 获取
ULocalPlayerSubsystem* LP = GetLocalPlayer()->GetSubsystem<ULocalPlayerSubsystem>();

// 方式 4：从 World 获取（WorldSubsystem 专用）
UMyWorldSubsystem* WS = GetWorld()->GetSubsystem<UMyWorldSubsystem>();
```

### 1.4 TickableWorldSubsystem（需要每帧更新的子系统）

```cpp
// MyTickableSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MyTickableSubsystem.generated.h"

UCLASS()
class MYGAME_API UMyTickableSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    // 必须实现：返回 Tick 的类型描述
    virtual TStatId GetStatId() const override;

    // 必须实现：每帧调用
    virtual void Tick(float DeltaTime) override;

    // 可选：是否启用 Tick
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
};
```

```cpp
// MyTickableSubsystem.cpp
#include "MyTickableSubsystem.h"

TStatId UMyTickableSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UMyTickableSubsystem, STATGROUP_Tickables);
}

void UMyTickableSubsystem::Tick(float DeltaTime)
{
    // 每帧逻辑
}

bool UMyTickableSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}
```

### 1.5 Subsystem 间依赖

使用 `Collection` 参数声明依赖关系，确保初始化顺序：

```cpp
void UMyGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // 声明依赖：UMyOtherSubsystem 必须先于本系统初始化
    Collection.InitializeDependency<UMyOtherSubsystem>();

    // 获取已初始化的依赖
    UMyOtherSubsystem* Other = Collection.GetSubsystem<UMyOtherSubsystem>();
    // ...
}
```

### 1.6 常见 Subsystem 实践场景

| 场景 | 推荐 Subsystem 类型 |
| --- | --- |
| 全局存档/成就 | `UGameInstanceSubsystem` |
| 关卡天气/时间系统 | `UWorldSubsystem` |
| 本地玩家输入配置 | `ULocalPlayerSubsystem` |
| 编辑器自定义工具 | `UEditorSubsystem` |
| 每帧更新的全局管理器 | `UTickableWorldSubsystem` |

---

## 二、多线程

### 2.1 UE 多线程基础概念

Unreal 使用**单游戏线程 + 任务线程池**架构：

| 线程 | 说明 |
| --- | --- |
| Game Thread | 游戏逻辑主循环，所有蓝图/C++ 游戏代码默认在此执行 |
| Render Thread | 渲染提交，由引擎自动管理 |
| Task Threads | 工作线程池，可调度异步任务 |

**核心规则**：非 Game Thread 的代码不能直接修改 UObject 属性、调用大部分引擎 API、访问大部分容器。

### 2.2 FRunnable —— 底层线程

最基础的线程接口，适用于需要完全控制线程生命周期的场景：

```cpp
// MyWorkerThread.h
#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"

class FMyWorkerThread : public FRunnable
{
public:
    FMyWorkerThread();
    virtual ~FMyWorkerThread();

    // FRunnable 接口
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

    // 外部控制
    void Start();
    void Shutdown();

    // 线程安全地获取结果
    bool GetResult(FString& OutResult);

private:
    FThreadSafeBool bShouldRun;
    FThreadSafeString Result;
    TUniquePtr<FRunnableThread> Thread;
};
```

```cpp
// MyWorkerThread.cpp
#include "MyWorkerThread.h"

FMyWorkerThread::FMyWorkerThread()
    : bShouldRun(false)
{
}

FMyWorkerThread::~FMyWorkerThread()
{
    Shutdown();
}

bool FMyWorkerThread::Init()
{
    return true;
}

uint32 FMyWorkerThread::Run()
{
    while (bShouldRun)
    {
        // 在工作线程中执行耗时操作（如文件 IO、网络请求等）
        FPlatformProcess::Sleep(0.1f);

        // 模拟耗时计算
        FString ComputedResult = TEXT("Computed");
        Result = ComputedResult;
    }
    return 0;
}

void FMyWorkerThread::Stop()
{
    bShouldRun = false;
}

void FMyWorkerThread::Start()
{
    bShouldRun = true;
    Thread = FRunnableThread::Create(this, TEXT("MyWorkerThread"), 0, TPri_BelowNormal);
}

void FMyWorkerThread::Shutdown()
{
    bShouldRun = false;
    if (Thread)
    {
        Thread->WaitForCompletion();
        Thread.Reset();
    }
}

bool FMyWorkerThread::GetResult(FString& OutResult)
{
    OutResult = Result.GetValue();
    return true;
}
```

使用方式：

```cpp
// 在 Actor 或其他类中
TSharedPtr<FMyWorkerThread> Worker = MakeShared<FMyWorkerThread>();
Worker->Start();

// ...

// 获取结果（线程安全）
FString Result;
Worker->GetResult(Result);

// 关闭
Worker->Shutdown();
```

### 2.3 FAsyncTask —— 可复用的异步任务

比 FRunnable 更高级，适合一次性或可复用的计算密集型任务：

```cpp
// MyAsyncTask.h
#pragma once

#include "CoreMinimal.h"
#include "Async/Async.h"

class FMyAsyncTask : public FNonAbandonableTask
{
    friend class FAutoDeleteAsyncTask<FMyAsyncTask>;

    FMyAsyncTask(int32 InValue)
        : InputValue(InValue), Result(0)
    {
    }

    void DoWork()
    {
        // 模拟耗时计算
        FPlatformProcess::Sleep(2.0f);
        Result = InputValue * InputValue;
    }

    FORCEINLINE TStatId GetStatId() const
    {
        RETURN_QUICK_DECLARE_CYCLE_STAT(FMyAsyncTask, STATGROUP_ThreadPoolAsyncTasks);
    }

    int32 InputValue;
    int32 Result;
};
```

使用方式：

```cpp
// 启动异步任务
auto* Task = new FAutoDeleteAsyncTask<FMyAsyncTask>(42);
Task->StartBackgroundTask();   // 在线程池中执行
// 或
Task->StartSynchronousTask();  // 在当前线程执行

// 检查是否完成（需要自己管理状态）
```

### 2.4 Async() 快捷函数 —— 最常用

适合简单的一次性异步任务，返回 `TFuture`：

```cpp
#include "Async/Async.h"

// 基本用法
TFuture<int32> Future = Async(EAsyncExecution::ThreadPool, []()
{
    // 在线程池中执行
    FPlatformProcess::Sleep(2.0f);
    return 42;
});

// 获取结果（会阻塞当前线程直到完成）
int32 Result = Future.Get();

// 带回调的异步
TFuture<int32> Future2 = Async(EAsyncExecution::ThreadPool, []()
{
    FPlatformProcess::Sleep(1.0f);
    return 100;
}, [&MyActor](TFuture<int32> ResultFuture)
{
    // 回调在 Game Thread 执行（使用 GameThread 异步执行）
    int32 Val = ResultFuture.Get();
    MyActor->SomeValue = Val;
});
```

### 2.5 FGraphEventRef —— 任务图（TaskGraph）

UE 的任务图系统允许声明任务依赖关系：

```cpp
#include "Async/TaskGraphInterfaces.h"

// 创建任务图事件
FGraphEventRef Task1 = FFunctionGraphTask::CreateAndDispatchWhenReady(
    []()
    {
        UE_LOG(LogTemp, Log, TEXT("Task 1 executed"));
    },
    TStatId{},  // 性能统计
    nullptr,    // 依赖的任务
    ENamedThreads::AnyBackgroundThreadNormalTask  // 目标线程
);

FGraphEventRef Task2 = FFunctionGraphTask::CreateAndDispatchWhenReady(
    []()
    {
        UE_LOG(LogTemp, Log, TEXT("Task 2 executed"));
    },
    TStatId{},
    &Task1,  // 依赖 Task1 完成
    ENamedThreads::GameThread  // 在 Game Thread 执行
);

// 等待所有任务完成
FTaskGraphInterface::Get().WaitUntilTaskCompletes(Task2);
```

### 2.6 线程安全容器

```cpp
#include "HAL/ThreadSafeCounter.h"
#include "HAL/ThreadSafe.h"

// 线程安全计数器
FThreadSafeCounter Counter;
Counter.Increment();
int32 Value = Counter.GetValue();

// 线程安全的原子操作
FThreadSafeCounter AtomicCounter;

// TQueue（无锁线程安全队列）
TQueue<int32> ThreadSafeQueue;
ThreadSafeQueue.Enqueue(42);
int32 OutValue;
ThreadSafeQueue.Dequeue(OutValue);

// FCriticalSection + FScopeLock（互斥锁）
FCriticalSection Mutex;

void SafeFunction()
{
    FScopeLock Lock(&Mutex);
    // 临界区代码
}
```

### 2.7 线程间通信：回到 Game Thread

从工作线程安全地执行 Game Thread 代码：

```cpp
// 方式 1：AsyncTask（最常用）
AsyncTask(ENamedThreads::GameThread, [WeakThis = TWeakObjectPtr<AMyActor>(this)]()
{
    if (AMyActor* Self = WeakThis.Get())
    {
        // 在 Game Thread 执行
        Self->UpdateUI();
    }
});

// 方式 2：FFunctionGraphTask
FFunctionGraphTask::CreateAndDispatchWhenReady(
    []()
    {
        // 在 Game Thread 执行
    },
    TStatId{},
    nullptr,
    ENamedThreads::GameThread
);

// 方式 3：FTimerManager 延迟（Game Thread）
GetWorldTimerManager().SetTimerForNextTick([WeakThis = TWeakObjectPtr<AMyActor>(this)]()
{
    if (AMyActor* Self = WeakThis.Get())
    {
        Self->OnAsyncComplete();
    }
});
```

---

## 三、异步操作

### 3.1 FTimerManager —— 定时器

比 Tick 更高效的定时执行机制：

```cpp
// 在 BeginPlay 中设置
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    // 单次延迟调用（2 秒后）
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &AMyActor::OnTimerFired,
        2.0f,     // 延迟时间
        false     // 是否循环
    );

    // 循环调用（每 0.5 秒）
    GetWorldTimerManager().SetTimer(
        LoopTimerHandle,
        this,
        &AMyActor::OnLoopTick,
        0.5f,
        true      // 循环
    );

    // 使用 Lambda
    GetWorldTimerManager().SetTimer(
        LambdaTimerHandle,
        [WeakThis = TWeakObjectPtr<AMyActor>(this)]()
        {
            if (AMyActor* Self = WeakThis.Get())
            {
                Self->DoSomething();
            }
        },
        1.0f,
        false
    );
}

void AMyActor::OnTimerFired()
{
    UE_LOG(LogTemp, Log, TEXT("Timer fired!"));
}

// 清除定时器
void AMyActor::StopTimer()
{
    GetWorldTimerManager().ClearTimer(TimerHandle);
    GetWorldTimerManager().ClearTimer(LoopTimerHandle);
}

// 暂停/恢复
GetWorldTimerManager().PauseTimer(LoopTimerHandle);
GetWorldTimerManager().UnPauseTimer(LoopTimerHandle);
```

### 3.2 Delay（蓝图等效的 C++ 实现）

```cpp
// 封装一个通用的 Delay 函数
void AMyActor::Delay(float Duration, TFunction<void()> Callback)
{
    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(
        Handle,
        [Callback = MoveTemp(Callback)]()
        {
            Callback();
        },
        Duration,
        false
    );
}

// 使用
Delay(3.0f, [WeakThis = TWeakObjectPtr<AMyActor>(this)]()
{
    if (AMyActor* Self = WeakThis.Get())
    {
        Self->OnDelayComplete();
    }
});
```

### 3.3 FStreamableManager —— 异步资源加载

加载大型资产（如 Texture、Mesh）时避免阻塞主线程：

```cpp
// Build.cs 添加模块依赖
PublicDependencyModuleNames.AddRange(new string[]
{
    "Core", "CoreUObject", "Engine", "AssetManager"
});
```

```cpp
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

// 异步加载单个资产
void AMyActor::LoadAssetAsync()
{
    FSoftObjectPath AssetPath(TEXT("/Game/Textures/MyTexture.MyTexture"));

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    StreamableRequestHandle = StreamableManager.RequestAsyncLoad(
        AssetPath,
        FStreamableDelegate::CreateUObject(this, &AMyActor::OnAssetLoaded)
    );
}

void AMyActor::OnAssetLoaded()
{
    if (StreamableRequestHandle.IsValid() && StreamableRequestHandle->HasLoadCompleted())
    {
        UTexture2D* LoadedTexture = Cast<UTexture2D>(StreamableRequestHandle->GetLoadedAsset());
        if (LoadedTexture)
        {
            // 使用加载完成的资源
        }
    }
}

// 异步加载多个资产
void AMyActor::LoadMultipleAssetsAsync()
{
    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(TEXT("/Game/Textures/T1.T1"));
    AssetsToLoad.Add(TEXT("/Game/Textures/T2.T2"));
    AssetsToLoad.Add(TEXT("/Game/Meshes/SM_MyMesh.SM_MyMesh"));

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    StreamableRequestHandle = StreamableManager.RequestAsyncLoad(
        AssetsToLoad,
        FStreamableDelegate::CreateLambda([WeakThis = TWeakObjectPtr<AMyActor>(this)]()
        {
            if (AMyActor* Self = WeakThis.Get())
            {
                // 所有资产加载完成
            }
        })
    );
}

// 使用 TSharedPtr 保持引用（防止加载中途被释放）
TSharedPtr<FStreamableHandle> StreamableHandle;

void AMyActor::LoadWithSharedHandle()
{
    FSoftObjectPath AssetPath(TEXT("/Game/Textures/MyTexture.MyTexture"));

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    StreamableHandle = StreamableManager.RequestAsyncLoad(
        AssetPath,
        FStreamableDelegate::CreateUObject(this, &AMyActor::OnAssetLoaded)
    );
}
```

### 3.4 FCoreUObjectDelegates —— 异步加载回调

```cpp
#include "UObject/UObjectGlobals.h"

// 监听所有资产加载完成
void AMyActor::SetupLoadDelegate()
{
    FCoreUObjectDelegates::AsyncLoadImage.AddLambda(
        [](const FString& FileName, UObject* LoadedObject, bool bSuccess)
        {
            if (bSuccess)
            {
                UE_LOG(LogTemp, Log, TEXT("Async image loaded: %s"), *FileName);
            }
        }
    );
}
```

### 3.5 Online Subsystem —— 异步网络请求

```cpp
// Build.cs
PublicDependencyModuleNames.AddRange(new string[]
{
    "OnlineSubsystem", "OnlineSubsystemUtils"
});

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineLeaderboardInterface.h"

void AMyActor::QueryLeaderboards()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
    if (!OnlineSub) return;

    IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
    if (!Leaderboards) return;

    FUniqueNetIdPtr PlayerId = GetGameInstance()->GetFirstLocalPlayerController()->PlayerState->GetUniqueId().GetUniqueNetId();

    Leaderboards->ReadLeaderboardsAroundRank(
        PlayerId.ToSharedRef(),
        10,  // 范围
        FOnLeaderboardReadComplete::CreateUObject(this, &AMyActor::OnLeaderboardRead)
    );
}

void AMyActor::OnLeaderboardRead(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        // 处理排行榜数据
    }
}
```

### 3.6 UE5 异步模式最佳实践

#### 原则 1：优先使用 Subsystem + Delegate

```cpp
// 在 Subsystem 中定义多步异步流程
void UMySubsystem::StartMultiStepProcess()
{
    Step1_LoadData()
        .Then([this](TFuture<bool> Result)
        {
            if (Result.Get())
            {
                Step2_ProcessData().Then([this](TFuture<bool> Result2)
                {
                    if (Result2.Get())
                    {
                        Step3_SaveData();
                    }
                });
            }
        });
}
```

#### 原则 2：使用弱引用防止悬垂

```cpp
// 错误：直接捕获 this
Async(EAsyncExecution::ThreadPool, [this]()
{
    // this 可能已被销毁
    SomeValue = 42;  // 危险！
});

// 正确：使用弱引用
Async(EAsyncExecution::ThreadPool, [WeakThis = TWeakObjectPtr<AMyActor>(this)]()
{
    if (AMyActor* Self = WeakThis.Get())
    {
        // 安全使用
        AsyncTask(ENamedThreads::GameThread, [Self]()
        {
            Self->SomeValue = 42;
        });
    }
});
```

#### 原则 3：资源清理

```cpp
void AMyActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清除定时器
    GetWorldTimerManager().ClearAllTimersForObject(this);

    // 取消异步资源加载
    if (StreamableRequestHandle.IsValid())
    {
        StreamableRequestHandle->CancelHandle();
        StreamableRequestHandle.Reset();
    }

    Super::EndPlay(EndPlayReason);
}
```

---

## 四、完整实战示例：异步加载 + Subsystem + 线程

### 场景：异步加载关卡资源并缓存到 GameInstance Subsystem

```cpp
// MyAssetCacheSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MyAssetCacheSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAssetCached, UObject*, LoadedAsset);

UCLASS()
class MYGAME_API UMyAssetCacheSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** 异步加载并缓存资源 */
    UFUNCTION(BlueprintCallable, Category = "AssetCache")
    void RequestAsset(const FSoftObjectPath& AssetPath);

    /** 同步获取已缓存资源（仅在已加载时有效） */
    UFUNCTION(BlueprintPure, Category = "AssetCache")
    UObject* GetCachedAsset(const FSoftObjectPath& AssetPath) const;

    /** 检查资源是否已缓存 */
    UFUNCTION(BlueprintPure, Category = "AssetCache")
    bool IsAssetCached(const FSoftObjectPath& AssetPath) const;

    /** 当资源加载完成时广播 */
    UPROPERTY(BlueprintAssignable, Category = "AssetCache")
    FOnAssetCached OnAssetCached;

private:
    void OnAsyncLoadComplete(const FSoftObjectPath& AssetPath);

    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> ActiveHandles;
    TMap<FSoftObjectPath, TWeakObjectPtr<UObject>> CachedAssets;
};
```

```cpp
// MyAssetCacheSubsystem.cpp
#include "MyAssetCacheSubsystem.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UMyAssetCacheSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("AssetCacheSubsystem initialized"));
}

void UMyAssetCacheSubsystem::Deinitialize()
{
    ActiveHandles.Empty();
    CachedAssets.Empty();
    Super::Deinitialize();
}

void UMyAssetCacheSubsystem::RequestAsset(const FSoftObjectPath& AssetPath)
{
    // 已缓存，直接返回
    if (IsAssetCached(AssetPath))
    {
        UObject* Cached = GetCachedAsset(AssetPath);
        OnAssetCached.Broadcast(Cached);
        return;
    }

    // 正在加载中
    if (ActiveHandles.Contains(AssetPath))
    {
        return;
    }

    FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();

    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        AssetPath,
        FStreamableDelegate::CreateUObject(
            this,
            &UMyAssetCacheSubsystem::OnAsyncLoadComplete,
            AssetPath
        )
    );

    ActiveHandles.Add(AssetPath, Handle);
}

UObject* UMyAssetCacheSubsystem::GetCachedAsset(const FSoftObjectPath& AssetPath) const
{
    if (const TWeakObjectPtr<UObject>* Found = CachedAssets.Find(AssetPath))
    {
        return Found->Get();
    }
    return nullptr;
}

bool UMyAssetCacheSubsystem::IsAssetCached(const FSoftObjectPath& AssetPath) const
{
    const TWeakObjectPtr<UObject>* Found = CachedAssets.Find(AssetPath);
    return Found && Found->IsValid();
}

void UMyAssetCacheSubsystem::OnAsyncLoadComplete(const FSoftObjectPath& AssetPath)
{
    if (TSharedPtr<FStreamableHandle>* Handle = ActiveHandles.Find(AssetPath))
    {
        if ((*Handle)->HasLoadCompleted())
        {
            UObject* LoadedAsset = (*Handle)->GetLoadedAsset();
            CachedAssets.Add(AssetPath, LoadedAsset);
            ActiveHandles.Remove(AssetPath);

            OnAssetCached.Broadcast(LoadedAsset);
        }
    }
}
```

**在 Actor 中使用**：

```cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    UMyAssetCacheSubsystem* Cache = GetGameInstance()->GetSubsystem<UMyAssetCacheSubsystem>();

    // 绑定加载完成回调
    Cache->OnAssetCached.AddDynamic(this, &AMyActor::OnAssetReady);

    // 请求异步加载
    Cache->RequestAsset(FSoftObjectPath(TEXT("/Game/Meshes/SM_Rock.SM_Rock")));
}

void AMyActor::OnAssetReady(UObject* LoadedAsset)
{
    if (UStaticMesh* Mesh = Cast<UStaticMesh>(LoadedAsset))
    {
        MeshComponent->SetStaticMesh(Mesh);
    }
}
```

---

## 五、常见问题与陷阱

| 问题 | 解决方案 |
| --- | --- |
| 工作线程中直接修改 UObject 属性 | 使用 `AsyncTask(ENamedThreads::GameThread, ...)` 回到主线程 |
| Subsystem 生命周期理解错误 | `GameInstanceSubsystem` 跨关卡持久，`WorldSubsystem` 随关卡销毁 |
| 异步加载的资源被 GC 回收 | 使用 `UPROPERTY()` 或 `TSharedPtr` 保持引用 |
| FRunnable 未正确停止 | 在析构函数中调用 `Shutdown()`，确保 `WaitForCompletion` |
| 多线程中访问 TArray/TMap | 使用 `FCriticalSection` + `FScopeLock` 或线程安全容器 |
| Delay/定时器在 Actor 销毁后触发 | 使用 `TWeakObjectPtr` 捕获，或在 `EndPlay` 中清除所有定时器 |
| Subsystem 间初始化顺序 | 使用 `Collection.InitializeDependency<T>()` 声明依赖 |

---

## 六、参考资源

- [Unreal Engine 官方文档 - Subsystems](https://docs.unrealengine.com/5.0/en-US/subsystems-in-unreal-engine/)
- [Unreal Engine 官方文档 - 异步任务](https://docs.unrealengine.com/5.0/en-US/async-task-in-unreal-engine/)
- [Unreal Engine 官方文档 - 线程](https://docs.unrealengine.com/5.0/en-US/multithreading-in-unreal-engine/)
- [Epic Games 开发者社区](https://dev.epicgames.com/community/)
