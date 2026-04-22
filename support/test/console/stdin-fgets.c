#include <stdio.h>

#include <windows.h>

static void puts_w(const wchar_t *s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	size_t len = wcslen(s);
	DWORD written;
	WriteConsoleW(hStdout, L"[note] ", 7, &written, NULL);
	WriteConsoleW(hStdout, s, len, &written, NULL);
	WriteConsoleW(hStdout, L"\n", 1, &written, NULL);
}

static void emu_input(const wchar_t *s)
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	size_t len = wcslen(s);
	INPUT_RECORD records[len * 2];
	for (size_t i = 0; i < len; i++) {
		BOOL is_enter = (s[i] == L'\n');
		records[i * 2].EventType = KEY_EVENT;
		records[i * 2].Event.KeyEvent.bKeyDown = TRUE;
		records[i * 2].Event.KeyEvent.wRepeatCount = 1;
		records[i * 2].Event.KeyEvent.wVirtualKeyCode = is_enter ? VK_RETURN : 0;
		records[i * 2].Event.KeyEvent.wVirtualScanCode = is_enter ? 28 : 0;
		records[i * 2].Event.KeyEvent.uChar.UnicodeChar = is_enter ? L'\r' : s[i];
		records[i * 2].Event.KeyEvent.dwControlKeyState = 0;
		records[i * 2 + 1] = records[i * 2];
		records[i * 2 + 1].Event.KeyEvent.bKeyDown = FALSE;
	}
	DWORD written;
	WriteConsoleInputW(hStdin, records, len * 2, &written);
}

int main(void)
{
	{
		char buf[80 + 1];

		puts_w(L"fgets short");
		emu_input(L"天地玄黄\n");
		fgets(buf, sizeof(buf), stdin);
		fputs(buf, stdout);
	}

	{
		char buf[200 + 1];

		puts_w(L"fgets long");
		emu_input(L"天地玄黄，宇宙洪荒；日月盈昃，辰宿列张；寒来暑往，秋收冬藏；闰馀成岁，律吕调阳；雲腾致雨，露结为霜；金生丽水，玉出崑冈；剑号巨阙，珠称夜光；果珍李柰，菜重芥姜；海咸河淡，鳞潜羽翔。龙师火帝，鸟官人皇；始制文字，乃服衣裳；推位让国，有虞陶唐；吊民伐罪，周發殷汤；坐朝问道，垂拱平章；爱育黎首，臣伏戎羌；遐迩一体，率宾归王；鸣凤在竹，白驹食场；化被草木，赖及万方。\n");
		fgets(buf, sizeof(buf), stdin);
		fputs(buf, stdout);
	}

	return 0;
}
