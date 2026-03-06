/*****************************************************************************
Linux/UNIX 下的 kbhit() 和 getch() 实现
Chris Giese <geezer@execpc.com>	http://my.execpc.com/~geezer
发布日期: ?
此代码为公共领域（无版权）。
您可以随意使用。
*****************************************************************************/
#if defined(_WIN32)
#include <conio.h> /* kbhit(), getch() */

#else
#include <sys/time.h> /* struct timeval, select() */
/* ICANON, ECHO, TCSANOW, struct termios */
#include <termios.h> /* tcgetattr(), tcsetattr() */
#include <cstdlib> /* atexit(), exit() */
#include <unistd.h> /* read() */
#include <cstdio> /* printf() */
#include <cstring> /* memcpy */

static struct termios g_old_kbd_mode;
/*****************************************************************************
恢复终端为正常（cooked）模式
*****************************************************************************/
static void cooked()
{
	tcsetattr(0, TCSANOW, &g_old_kbd_mode);
}
/*****************************************************************************
设置终端为原始（raw）模式
*****************************************************************************/
static void raw()
{
	static char init;
/**/
	struct termios new_kbd_mode;

	if(init)
		return;
/* 将键盘（实际上是标准输入）设置为原始、无缓冲模式 */
	tcgetattr(0, &g_old_kbd_mode);
	memcpy(&new_kbd_mode, &g_old_kbd_mode, sizeof(struct termios));
	new_kbd_mode.c_lflag &= ~(ICANON /*| ECHO */ );
	new_kbd_mode.c_cc[VTIME] = 0;
	new_kbd_mode.c_cc[VMIN] = 1;
	tcsetattr(0, TCSANOW, &new_kbd_mode);
/* 退出时恢复正常的 "cooked" 模式 */
	atexit(cooked);

	init = 1;
}
/*****************************************************************************
检测是否有按键输入（非阻塞）
*****************************************************************************/
static int kbhit()
{
	struct timeval timeout;
	fd_set read_handles;
	int status;

	raw();
/* 检查标准输入（文件描述符 0）是否有活动 */
	FD_ZERO(&read_handles);
	FD_SET(0, &read_handles);
	timeout.tv_sec = timeout.tv_usec = 0;
	status = select(0 + 1, &read_handles, nullptr, nullptr, &timeout);
	if(status < 0)
	{
		printf("kbhit() 中 select() 调用失败\n");
		exit(1);
	}
	return status;
}
/*****************************************************************************
读取单个字符（不回显）
*****************************************************************************/
static int getch()
{
	unsigned char temp;

	raw();
/* 标准输入的文件描述符为 0 */
	if(read(0, &temp, 1) != 1)
		return 0;
	return temp;
}
#endif


