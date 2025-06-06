const WebSocket = require('ws');

// 默认配置
const DEFAULT_IP = '127.0.0.1';
const DEFAULT_PORT = 6666;
const DEFAULT_CHECK_INTERVAL = 16;

const logQueue = []; // 使用数组作为日志队列

// 从命令行参数获取 IP、Port 和 CheckInterval
const ip = process.argv[2] || DEFAULT_IP; // 默认 IP
const port = process.argv[3] ? parseInt(process.argv[3], 10) : DEFAULT_PORT; // 默认 Port
const checkInterval = process.argv[4] ? parseInt(process.argv[4], 10) : DEFAULT_CHECK_INTERVAL; // 默认 CheckInterval

// 日志等级到颜色的映射
const logColors = {
    0: '\x1b[37m', // 白色
    1: '\x1b[33m', // 黄色
    2: '\x1b[31m', // 红色
    3: '\x1b[32m', // 绿色
};

// 日志等级重置为默认
const resetColor = '\x1b[0m';

// 创建 WebSocket 服务器
const wss = new WebSocket.Server({ host: ip, port: port });

// 处理连接
wss.on('connection', (ws) => {
    console.log(`${logColors[3]}Client connected${resetColor}`); 

    // 处理接收到的消息
    ws.on('message', (message) => {
        //console.log(`Received raw message: ${message}`); // 打印接收到的原始消息
        try {
            const logMessage = JSON.parse(message);
            if (logMessage.level !== undefined && logMessage.content) {
                logQueue.push(logMessage); // 将消息放入队列
            } else {
                console.error('Invalid log message format');
            }
        } catch (error) {
            console.error('Invalid message format', error);
        }
    });

    // 处理连接关闭
    ws.on('close', () => {
        console.log(`${logColors[3]}Client disconnected${resetColor}`); 
    });
});

// 异步处理日志队列
async function processLogQueue() {
    setInterval(() => {
        if (logQueue.length > 0) {
            const logMessage = logQueue.shift(); // 从队列中取出消息
            const { time, level, content } = logMessage;
            // 打印日志，使用时间戳
            console.log(`${logColors[3]}${time} ${logColors[level]}[${getLogLevelString(level)}] ${content}${resetColor}`);
        }
    }, checkInterval);
}

// 获取日志等级字符串
function getLogLevelString(level) {
    switch (level) {
        case 0: return 'Info';
        case 1: return 'Warn';
        case 2: return 'Error';
        default: return 'Info';
    }
}

// 启动日志处理
processLogQueue();

console.log(`${logColors[3]}WebSocket server is running on ws://${ip}:${port}${resetColor}`);

// 处理进程关闭
process.on('SIGINT', () => {
    console.log(`${logColors[3]}Closing WebSocket server...${resetColor}`); 
    wss.close(() => {
        console.log(`${logColors[3]}WebSocket server closed.${resetColor}`);
        process.exit(0);
    });
});
