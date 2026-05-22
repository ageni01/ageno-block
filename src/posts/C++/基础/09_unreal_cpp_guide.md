# Unreal C++ 详细入门与工程说明

## 1. 文档目标

本文面向 Unreal Engine C++ 开发，整理最常用的核心知识：

- Unreal C++ 与标准 C++ 的关系
- 反射宏
- `UObject` / `AActor` / `UActorComponent`
- 属性系统
- 常用容器
- 生命周期
- 事件与输入
- 网络同步基础

说明：

- Unreal C++ 不是“纯标准 C++ 项目”
- 它有自己的对象系统、反射系统、构建工具和宏体系

---

## 2. Unreal C++ 与标准 C++ 的区别

Unreal C++ = 标准 C++ + Unreal 引擎框架 + Unreal Header Tool 反射系统。

特点：

- 大量使用宏
- 对象创建通常不用直接 `new`
- 垃圾回收与 `UObject` 生命周期强相关
- 与编辑器、蓝图、资源系统深度绑定

---

## 3. 常见基类

### 3.1 `UObject`

最基础的 Unreal 对象类型。

适合：

- 非场景对象
- 数据对象
- 工具对象

### 3.2 `AActor`

可放入关卡中的对象。

适合：

- 角色
- 可交互物体
- 场景实体

### 3.3 `UActorComponent`

组件化功能单元。

适合：

- 移动逻辑
- 战斗逻辑
- 感知系统
- 自定义行为模块

---

## 4. 反射宏

Unreal 的核心之一是反射宏。

### 4.1 `UCLASS`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyActor.generated.h"

UCLASS()
class MYGAME_API AMyActor : public AActor {
    GENERATED_BODY()

public:
    AMyActor();
};
```

### 4.2 `UPROPERTY`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
float MoveSpeed = 300.0f;
```

作用：

- 暴露给编辑器
- 暴露给蓝图
- 纳入反射/序列化/GC 跟踪体系

### 4.3 `UFUNCTION`

```cpp
UFUNCTION(BlueprintCallable, Category = "Action")
void Fire();
```

作用：

- 暴露给蓝图调用
- 支持反射系统

### 4.4 `USTRUCT`

```cpp
USTRUCT(BlueprintType)
struct FItemData {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
};
```

---

## 5. 基础类示例：Actor

### 5.1 头文件

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DemoActor.generated.h"

UCLASS()
class MYGAME_API ADemoActor : public AActor {
    GENERATED_BODY()

public:
    ADemoActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
    float Speed = 200.0f;
};
```

### 5.2 源文件

```cpp
#include "DemoActor.h"

ADemoActor::ADemoActor() {
    PrimaryActorTick.bCanEverTick = true;
}

void ADemoActor::BeginPlay() {
    Super::BeginPlay();
}

void ADemoActor::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
}
```

---

## 6. 常用类型

### 6.1 `FString`

```cpp
FString Name = TEXT("Player");
```

### 6.2 `FName`

适合：

- 标识符
- 标签
- 频繁比较

```cpp
FName SocketName(TEXT("WeaponSocket"));
```

### 6.3 `FText`

适合：

- 面向玩家的可本地化文本

```cpp
FText Title = FText::FromString(TEXT("Start Game"));
```

---

## 7. 常用容器

### 7.1 `TArray`

```cpp
TArray<int32> Ids;
Ids.Add(1);
Ids.Add(2);
```

### 7.2 `TMap`

```cpp
TMap<FString, int32> Scores;
Scores.Add(TEXT("Alice"), 100);
```

### 7.3 `TSet`

```cpp
TSet<int32> UniqueIds;
UniqueIds.Add(10);
UniqueIds.Add(10);
```

---

## 8. 对象创建与生命周期

### 8.1 不要对 `UObject` 直接 `new`

通常应使用：

- `NewObject<T>()`
- `CreateDefaultSubobject<T>()`
- `SpawnActor<T>()`

### 8.2 创建 `UObject`

```cpp
UMyObject* Obj = NewObject<UMyObject>(this);
```

### 8.3 创建组件

```cpp
RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
```

### 8.4 生成 Actor

```cpp
FActorSpawnParameters Params;
ADemoActor* Actor = GetWorld()->SpawnActor<ADemoActor>(ADemoActor::StaticClass(), Params);
```

---

## 9. `UPROPERTY` 与垃圾回收

如果一个 `UObject*` 成员需要被引擎正确跟踪，通常要写成：

```cpp
UPROPERTY()
UMyObject* MyObject;
```

否则可能出现：

- GC 无法跟踪
- 指针悬空
- 编辑器或运行时异常

---

## 10. 输入绑定

### 10.1 旧输入系统示例

```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
}
```

### 10.2 Enhanced Input

实际项目中通常建议使用 Enhanced Input 系统，并把输入映射独立配置。

---

## 11. 组件化开发

推荐把复杂角色逻辑拆成组件，例如：

- `UHealthComponent`
- `UInventoryComponent`
- `UInteractionComponent`

示例：

```cpp
UPROPERTY(VisibleAnywhere)
UStaticMeshComponent* MeshComp;
```

---

## 12. 蓝图与 C++ 协作

推荐模式：

- 核心逻辑用 C++
- 参数配置和简单组合用蓝图
- 事件入口暴露给蓝图

示例：

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "UI")
void OnHpChanged(float NewHp);
```

---

## 13. 网络同步基础

### 13.1 属性复制

```cpp
UPROPERTY(Replicated)
int32 Hp = 100;
```

### 13.2 RPC

```cpp
UFUNCTION(Server, Reliable)
void ServerFire();
```

```cpp
UFUNCTION(NetMulticast, Unreliable)
void MulticastPlayEffect();
```

说明：

- `Server`：客户端请求服务器执行
- `Client`：服务器发给指定客户端
- `NetMulticast`：服务器广播给所有客户端

---

## 14. Unreal 智能指针

Unreal 里除了标准库智能指针，还有自己的指针体系：

- `TSharedPtr`
- `TWeakPtr`
- `TUniquePtr`
- `TObjectPtr`
- `TWeakObjectPtr`
- `TSoftObjectPtr`

### 14.1 `TSharedPtr`

适合非 `UObject` 普通 C++ 对象。

```cpp
TSharedPtr<int32> Value = MakeShared<int32>(10);
```

### 14.2 `TWeakObjectPtr`

适合弱引用 `UObject`。

```cpp
TWeakObjectPtr<AActor> CachedActor;
```

### 14.3 `TSoftObjectPtr`

适合资源延迟加载引用。

```cpp
UPROPERTY(EditAnywhere)
TSoftObjectPtr<UTexture2D> IconTexture;
```

---

## 15. 常见坑

1. 忘记 `.generated.h`
2. `UObject` 用裸 `new`
3. `UObject*` 成员没加 `UPROPERTY`
4. 在构造阶段访问运行时世界对象
5. 把所有逻辑全塞进一个 `Actor`
6. 不区分 `FString`、`FName`、`FText`

---

## 16. 学习建议

建议顺序：

1. 先掌握标准 C++ 基础
2. 再理解 `UObject` / `Actor` / `Component`
3. 再学反射宏和属性系统
4. 再学蓝图协作
5. 再进入网络同步、Gameplay Framework、AI、动画系统

---

## 17. Unreal C++ 和普通 C++ 的边界

要记住两件事：

1. Unreal C++ 仍然是 C++，语法基础不会变
2. 但项目结构、内存管理、反射、资源系统都强烈依赖引擎规则

所以写 Unreal 代码时，不能只按标准库思路写，还要遵守引擎对象生命周期和反射机制。

