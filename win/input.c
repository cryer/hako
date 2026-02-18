#define _WIN32_WINNT 0x0600  // 确保包含 SendInput 所需的定义
#include <windows.h>
#include <stdio.h>

/* 
 * 宏定义：屏幕坐标归一化
 * SendInput 使用绝对坐标时，需要将像素坐标映射到 0-65535 范围
 */
#define MOUSE_COORD_TO_ABS(coord, width_or_height) ( \
    (65536 * (coord) / (width_or_height)) + ((coord) < 0 ? -1 : 1) \
)

/**
 * @brief 移动鼠标到指定屏幕绝对坐标
 * @param x 屏幕 X 像素坐标
 * @param y 屏幕 Y 像素坐标
 */
void MouseMoveTo(int x, int y) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    
    // 获取屏幕分辨率，用于坐标归一化
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 设置鼠标事件标志：绝对坐标 + 移动
    input.mi.dx = MOUSE_COORD_TO_ABS(x, screenWidth);
    input.mi.dy = MOUSE_COORD_TO_ABS(y, screenHeight);
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

    // 发送输入事件
    SendInput(1, &input, sizeof(INPUT));
}

/**
 * @brief 模拟鼠标点击
 * @param x 点击位置 X
 * @param y 点击位置 Y
 * @param isLeft true 为左键，false 为右键
 */
void MouseClick(int x, int y, BOOL isLeft) {
    // 先移动过去
    MouseMoveTo(x, y);
    Sleep(10); // 微小延迟，确保移动完成

    INPUT inputs[2] = {0};

    // 按下事件
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = isLeft ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;

    // 抬起事件
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = isLeft ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;

    // 一次性发送按下和抬起
    SendInput(2, inputs, sizeof(INPUT));
}

/**
 * @brief 模拟物理按键
 * 使用虚拟键码 (VK)，依赖键盘布局，不适合输入文本
 */
void KeyTapVK(WORD vkCode) {
    INPUT inputs[2] = {0};

    // 按下
    inputs[0].type = INPUT_KEYBOARD;
    // 设置虚拟键码 按键必须使用虚拟键码
    // 文本模式可以使用unicode，wVK设为0
    inputs[0].ki.wVk = vkCode;
    // 设置标志位：如果是抬起，需要加上 KEYEVENTF_KEYUP
    inputs[0].ki.dwFlags = 0;
    // 注意：现代 Windows 通常不需要 KEYEVENTF_SCANCODE，除非模拟特殊硬件键
    // 使用 wVk (虚拟键码) 兼容性更好

    // 抬起
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = vkCode;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

/**
 * @brief 模拟文本字符输入 而不是物理按键
 * 使用 Unicode 模式，绕过输入法和键盘布局，最稳定，只适合聊天框文本，不能作为角色控制
 * @param wchar 要输入的宽字符，例如 L'A'
 */
void KeyTapChar(wchar_t wchar) {
    INPUT inputs[2] = {0};

    // 按下
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = 0; // 必须为 0
    inputs[0].ki.wScan = wchar; // 放入 Unicode 字符
    inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

    // 抬起
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 0;
    inputs[1].ki.wScan = wchar;
    inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

/**
 * @brief 模拟输入字符串
 * 模拟字符生成事件
 */
void TypeText(const wchar_t* text) {
    while (*text) {
        KeyTapChar(*text);
        Sleep(50); // 模拟打字间隔，太快可能被某些程序忽略
        text++;
    }
}


int main() {
    printf("=== Windows 原生输入模拟演示 ===\n");
    printf("警告：程序将控制您的鼠标和键盘！\n");
    printf("5 秒后开始执行...\n");
    Sleep(5000);

    // 1. 测试鼠标移动
    printf("1. 移动鼠标到屏幕中心...\n");
    int centerX = GetSystemMetrics(SM_CXSCREEN) / 2;
    int centerY = GetSystemMetrics(SM_CYSCREEN) / 2;
    MouseMoveTo(centerX, centerY);
    Sleep(1000);

    // 2. 测试鼠标点击
    printf("2. 在屏幕中心左键点击...\n");
    MouseClick(centerX, centerY, TRUE);
    Sleep(1000);

    // 3. 测试键盘输入
    printf("3. 模拟输入 'Hello' (请确保焦点在文本框)...\n");
    // 使用 Unicode 模式输入文本，不受输入法影响
    TypeText(L"Hello World");

    // 使用 VK 模式输入功能键
    // KeyTapVK(VK_RETURN);

    printf("演示结束。\n");
    return 0;
}
