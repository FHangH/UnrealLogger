# Unreal Logger

1. 一个可跨平台，简单，轻量，易维护的UE日志工具
2. 解决UE项目打包发行版后，查看项目中手动添加的PrintLog等相关操作，无法轻易查看日志的问题
3. 基于WebSocket通信，可以本地使用，也可以部署云端



### 1. 结构介绍

UnrealLogger

- UE_Logger
  - node_modules
  - logger.js `Logger Server`
  - package.json
  - package-lock.json
  - websocat.exe `Windows Test Websocket Tool`
- UnrealLogger `UE Plugin`
- .gitignore
- README.md



`Logger Server`：使用`nodejs`开发

`UE Plugin`：使用`UE 5.2`开发，纯源码，需要使用C++项目编译，只用了`UE`模块，理论上该插件也支持全平台，足够多的UE版本

`websocat.exe`：启动`Logger Server`后，在`Windows`平台，可以简单使用该测试是否启用成功



### 2. 使用前提

1. 系统运行环境需要`nodejs, npm`等
2. UE项目是C++项目，添加该插件后，项目编译成功



### 3. 使用方法

#### 部署LoggerServer

1. 使用控制台进入`UE_Logger`目录

2. 运行`node logger.js`启动 `Logger Server`

   - 默认启动的是 `127.0.0.1:6666`, 刷新间隔 `16ms`
   - 使用带参命令 `node logger.js 127.0.0.1 7777 100`可以该`Logger Server`的`IP, Port, 刷新间隔`
   - `node logger.js <IP> <Port> <Interval>`

3. 启动后，在`Windows`环境下，可以直接使用`websocat.exe`测试

   - 建议新开一个控制台，同样的目录下`websocat.exe ws://127.0.0.1:6666`

     ![](README.assets/image-20241016110348961.png)

   - 测试数据：

     - `{"level":0, "content":"这是普通日志"}`
     - `{"level":1, "content":"这是警告日志"}`
     - `{"level":2, "content":"这是错误日志"}`

     ![image-20241016110715719](README.assets/image-20241016110715719.png)



#### UE 插件使用

>注意：使用前，先确保 Logger Server已经启动，并测试通过
>
>如果：先启动了UE 客户端，再启动 Logger Server，应该是无法连接的，可以在客户端程序中，可以手动或特定时机触发 MakeLoggerSetting方法



1. 初始化 `Logger System`：初始化相关配置和启动`WebSocket`服务

   ![image-20241016111135588](README.assets/image-20241016111135588.png)

2. 查看`PrintLog`方法

   ![image-20241016111241741](README.assets/image-20241016111241741.png)

3. `Log Type`介绍：

   - Normal -> level: 0
   - Warning -> level: 1
   - Error -> level:2

4. `Log Setting`：

   ![image-20241016111441276](README.assets/image-20241016111441276.png)

5. `Log Screen Color`，`Log Screen Time`和 `UE printString`同理

6. 测试蓝图：

   ![image-20241016111612309](README.assets/image-20241016111612309.png)

   运行结果：

   ![image-20241016111743694](README.assets/image-20241016111743694.png)

   ![image-20241016111755695](README.assets/image-20241016111755695.png)

7. 结束打印

   ![image-20241016111826499](README.assets/image-20241016111826499.png)

![image-20241016111842718](README.assets/image-20241016111842718.png)



### 4. UE客户端可能出现的问题

![](README.assets/image-20241016111643096.png)

如果你电脑挂了代理，这个应该是代理的问题，关掉代理再测试



### 5. 云端部署

1. 测试了一个Linux服务器，开发防火墙的`6666`端口
2. 配置运行环境
3. `node logger.js 0.0.0.0:6666`
4. 在本地主机（windows）: `websocat.exe ws://公网IP:6666`即可

![image-20241017174733239](README.assets/image-20241017174733239.png)



### 补充更新

#### 2024-10-16

![image-20241016153058664](README.assets/image-20241016153058664.png)

多了一个 `IsUseWorldContextName`，目的是方便查看日志打印时，可以看见这条日志打印的所在位置，比如：`UI, Actor, Level`等等；

优化了`Logger Server`的打印样式，使用绿色显示连接状态

![image-20241016153349930](README.assets/image-20241016153349930.png)

![image-20241016153454816](README.assets/image-20241016153454816.png)



#### 2024-10-18

发送给`Logger Server`的日志，默认增加了时间戳 `UE 发送日志的时间`

![image-20241018153952526](README.assets/image-20241018153952526.png)



完善蓝图的使用体验，默认`（WorldContextObject=Self, IsUseWorldContextName=True, LogType=Normal, Setting="Logger", LogScreenColor="0.0f, 0.66f, 1.0f, 1.0f", LogScreenTime=2.0f）`

节点默认只显示`LogText`，`LogSetting`不再是结构体，而是默认折叠起来，和`UE蓝图 printString节点相同`

![image-20241018173231908](README.assets/image-20241018173231908.png)



#### 2024-10-28

优化了 `PrintLog`节点默认的展开参数为：`LogType`, `LogText`，方便快速设置打印日志的类型，打印目标默认为`Screen, UE_LOG, UnrealLogger`
修复了，传入`Json`字符串，无法正确解析的问题（需要对传入字符内，所有的 `""`做转义）



#### 2024-10-29

1. 修复打印`json`的相关问题
2. 新增节点`SetGlobalSetting`，可以全局控制打印日志到何处



#### 2024-10-31

经过一段时间的使用，发现`LogType`还是默认`UE_Log-Logger`比较舒服



#### 2025-03-05

![image-20250305160320327](README.assets/image-20250305160320327.png)

修改节点`MakeLoggerSetting`

新增字段：

- `IsEnableQueueMode`：是否开启队列模式
- `QueueCheckIntervalSeconds`：队列检查的间隔时间



此前在`GameInstance`中的`Init()`使用，可能时机过早，丢失了日志信息；

加入队列模式后，同样的情况下，日志不会丢失，而是进入队列中，当初始化完毕可以正常使用之后的新打印，会触发队列检查，将此前无法正常打印的信息打印出来



总之：在`GameInstance`的`Init()`之后`MakeLoggerSetting()`时机过早；
解决方案：

- 在自定义的`GameInstance.cpp`中声明`OnPostInit`

  ```c++
  UCLASS()
  class TEST_TUTORIAL_API UFH_GameInstance : public UGameInstance
  {
  	GENERATED_BODY()
  
  protected:
  	virtual void Init() override;
  
  	UFUNCTION(BlueprintNativeEvent, Category="FH|GameInstance")
  	void OnPostInit();
  };
  ```

- 在`Init()`中调用`OnPostInit()`

  ```c++
  void UFH_GameInstance::Init()
  {
  	Super::Init();
  
  	OnPostInit();
  }
  
  void UFH_GameInstance::OnPostInit_Implementation()
  {
  }
  ```

- 在蓝图中定义：`OnPostInit()`

- 建议在这之后进行游戏逻辑

![image-20250305162119649](README.assets/image-20250305162119649.png)

这样使用，比较靠谱，如果是在`GameMode`,`PlayerController`或其它`A*`的类型对象中使用，则不必担心这个问题
