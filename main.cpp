#define UNICODE

#include <windows.h>
#include "resource.h"

TCHAR szClassName[]=TEXT("Reversi");

#define MASU_SIZE   50                       //マス目（石）のサイズ
#define MASU_NUM    8                        //マス目の数（１方向に対する）
#define BOARD_SIZE  MASU_SIZE*MASU_NUM     //オセロ板のサイズ
#define SYSTEM_SIZE 100                      //制御部分のサイズ
#define WINDOW_W    BOARD_SIZE //+ SYSTEM_SIZE //ウィンドウの横幅
#define WINDOW_H    BOARD_SIZE               //ウィンドウの縦幅
#define BLACK_STONE   1                      //黒石
#define WHITE_STONE  -1                      //白石
#define BLANK         0                      //石なし
#define END_NUMBER   60                      //オセロ終了の手数
#define SEARCH_LV     5                      //探索する手数
#define MIDDLE_NUM   10                      //中盤の始まる手数
#define FINISH_NUM   48                      //終盤の始まる手数

const int ValuePlace[MASU_NUM][MASU_NUM]=
{
	{ 45,-11,4,-1,-1,4,-11,45,},
	{-11,-16,-1,-3,-3,-1,-16,-11,},
	{  4,-1,2,-1,-1,2,-1,4,},
	{ -1,-3,-1,0,0,-1,-3,-1,},
	{ -1,-3,-1,0,0,-1,-3,-1,},
	{  4,-1,2,-1,-1,2,-1,4,},
	{-11,-16,-1,-3,-3,-1,-16,-11,},
	{ 45,-11,4,-1,-1,4,-11,45,}
};

typedef struct reverse_info
{
	int x,y;
	int pointer;
	int position[30];
}Ando;

int m_Board[MASU_NUM][MASU_NUM];
int m_PutNumber;
int m_SearchLv;
BOOL m_FlagForWhite;
BOOL m_FlagForPlayer;
BOOL m_FlagInGame;
HWND hWnd;
HANDLE hThread1;
BOOL g_ThreadFlag=FALSE;

int CountStone(int stone)
{
	int x,y,count=0;
	for(x=0;x<MASU_NUM;x++)
		for(y=0;y<MASU_NUM;y++)
			if(m_Board[x][y]==stone)
				count++;
	return(count);
}

void End()
{
	if(m_PutNumber==END_NUMBER)
	{
		int num;
		if(m_FlagForPlayer)
			num=CountStone(WHITE_STONE);
		else
			num=CountStone(BLACK_STONE);
		m_FlagInGame=FALSE;
		if(num*2>(m_PutNumber+4))
			MessageBox(
			hWnd,
			TEXT("あなたの勝ちです"),
			TEXT("勝敗"),
			0);
		else if(num*2<(m_PutNumber+4))
			MessageBox(
			hWnd,
			TEXT("あなたの負けです"),
			TEXT("勝敗"),
			0);
		else 
			MessageBox(
			hWnd,
			TEXT("引き分けです"),
			TEXT("勝敗"),
			0);
	}
}

void InitBoard()
{
	int x,y;
	for(x=0;x<MASU_NUM;x++)
		for(y=0;y<MASU_NUM;y++)
			m_Board[x][y]=BLANK;
	m_Board[3][3]=m_Board[4][4]=WHITE_STONE;
	m_Board[3][4]=m_Board[4][3]=BLACK_STONE;
	m_FlagForWhite=FALSE;
	m_PutNumber=0;
}

void DrawBoard(HDC hdc)
{
	int x,y;
	HBRUSH green_brush,black_brush,white_brush,old_brush;
	white_brush=(HBRUSH)GetStockObject(WHITE_BRUSH);
	green_brush=CreateSolidBrush(RGB(0,128,0));
	black_brush=CreateSolidBrush(RGB(0,0,0));
	old_brush=(HBRUSH)SelectObject(hdc,green_brush);
	for(x=0;x<MASU_NUM;x++)
	{
		for(y=0;y<MASU_NUM;y++)
		{
			Rectangle(
				hdc,
				x*MASU_SIZE,
				y*MASU_SIZE,
				(x+1)*MASU_SIZE-1,
				(y+1)*MASU_SIZE-1);
			if(m_Board[x][y]==BLANK)
				continue;
			else if(m_Board[x][y]==BLACK_STONE)
				SelectObject(hdc,black_brush);
			else SelectObject(hdc,white_brush);
			Ellipse(
				hdc,
				x*MASU_SIZE+3,
				y*MASU_SIZE+3,
				(x+1)*MASU_SIZE-4,
				(y+1)*MASU_SIZE-4);
			SelectObject(hdc,green_brush);
		}
	}
	SelectObject(hdc,old_brush);
	DeleteObject(green_brush);
	DeleteObject(black_brush);
	DeleteObject(white_brush);
	if(m_FlagInGame)End();
}

void ReReverse(Ando ando)
{
	int i=0;
	while(ando.position[i]!=-1)
	{
		m_Board[ando.position[i]%MASU_NUM][ando.position[i]/MASU_NUM]*=-1;
		i++;
	}
	m_Board[ando.x][ando.y]=BLANK;
	m_FlagForWhite=!m_FlagForWhite;
}

BOOL CanDropDown(int x,int y,int vect_x,int vect_y)
{
	int put_stone;
	if(m_FlagForWhite)put_stone=WHITE_STONE;
	else put_stone=BLACK_STONE;
	x+=vect_x;
	y+=vect_y;
	if(x<0||x>=MASU_NUM||y<0||y>=MASU_NUM)return(FALSE);
	if(m_Board[x][y]==put_stone)return(FALSE);
	if(m_Board[x][y]==BLANK)return(FALSE);
	x+=vect_x;
	y+=vect_y;
	while(x>=0&&x<MASU_NUM&&y>=0&&y<MASU_NUM)
	{
		if(m_Board[x][y]==BLANK)return(FALSE);
		if(m_Board[x][y]==put_stone)return(TRUE);
		x+=vect_x;
		y+=vect_y;
	}
	return(FALSE);
}

BOOL CanDropDown(int x,int y)
{
	if(x>=MASU_NUM||y>=MASU_NUM)return(FALSE);
	if(m_Board[x][y]!=BLANK)return(FALSE);
	if(CanDropDown(x,y,1,0))return(TRUE);
	if(CanDropDown(x,y,0,1))return(TRUE);
	if(CanDropDown(x,y,-1,0))return(TRUE);
	if(CanDropDown(x,y,0,-1))return(TRUE);
	if(CanDropDown(x,y,1,1))return(TRUE);
	if(CanDropDown(x,y,-1,-1))return(TRUE);
	if(CanDropDown(x,y,1,-1))return(TRUE);
	if(CanDropDown(x,y,-1,1))return(TRUE);
	return(FALSE);
}

int ValueBoardNumber()
{
	int x,y,value=0;
	for(x=0;x<MASU_NUM;x++)
		for(y=0;y<MASU_NUM;y++)
			value+=m_Board[x][y];
	return(value*-1);
}

int ValueBoardDropDownNum()
{
	int x,y,value=0;
	for(x=0;x<MASU_NUM;x++)
		for(y=0;y<MASU_NUM;y++)
			if(CanDropDown(x,y))value+=1;
	if(m_FlagForWhite==!m_FlagForPlayer)
		return(3*value);
	else return(-3*value);
}

int ValueBoardPlace()
{
	int x,y,value=0;
	for(x=0;x<MASU_NUM;x++)
		for(y=0;y<MASU_NUM;y++)
			value+=m_Board[x][y]*ValuePlace[x][y];
	return(-value);
}

int ValueBoard()
{
	int value=0;
	if(m_PutNumber<=MIDDLE_NUM)
	{
		value+=ValueBoardPlace();
		value+=ValueBoardDropDownNum();
	}
	else if(m_PutNumber<=FINISH_NUM)
	{
		value+=ValueBoardPlace();
		value+=ValueBoardDropDownNum();
	}
	else value+=ValueBoardNumber();
	if(!m_FlagForPlayer)return(value);
	else return(-value);
}

void InitAndo(Ando*p_ando,int x,int y)
{
	p_ando->x=x;
	p_ando->y=y;
	p_ando->pointer=0;
	for(int i=0;i<30;i++)p_ando->position[i]=-1;
}

void Reverse(Ando*p_ando,int vect_x,int vect_y)
{
	int put_stone;
	int x=p_ando->x;
	int y=p_ando->y;
	int i=p_ando->pointer;
	if(m_FlagForWhite)put_stone=WHITE_STONE;
	else put_stone=BLACK_STONE;
	while(m_Board[x+=vect_x][y+=vect_y]!=put_stone)
	{
		m_Board[x][y]=put_stone;
		p_ando->position[i++]=x+y*MASU_NUM;
	}
	p_ando->position[p_ando->pointer=i]=-1;
}

void Reverse(Ando*p_ando)
{
	if(CanDropDown(p_ando->x,p_ando->y,1,0))
		Reverse(p_ando,1,0);
	if(CanDropDown(p_ando->x,p_ando->y,0,1))
		Reverse(p_ando,0,1);
	if(CanDropDown(p_ando->x,p_ando->y,-1,0))
		Reverse(p_ando,-1,0);
	if(CanDropDown(p_ando->x,p_ando->y,0,-1))
		Reverse(p_ando,0,-1);
	if(CanDropDown(p_ando->x,p_ando->y,1,1))
		Reverse(p_ando,1,1);
	if(CanDropDown(p_ando->x,p_ando->y,-1,-1))
		Reverse(p_ando,-1,-1);
	if(CanDropDown(p_ando->x,p_ando->y,1,-1))
		Reverse(p_ando,1,-1);
	if(CanDropDown(p_ando->x,p_ando->y,-1,1))
		Reverse(p_ando,-1,1);
}

void DropDownStone(int x,int y)
{
	int stone;
	if(m_FlagForWhite)stone=WHITE_STONE;
	else stone=BLACK_STONE;
	m_Board[x][y]=stone;
	m_FlagForWhite=!m_FlagForWhite;
}

int Min_Max(BOOL Flag,int lv,BOOL Put,int alpha,int beta)
{
	int  temp,x,y,vest_x,vest_y;
	BOOL FlagForPut=FALSE;
	Ando ando;
	if(lv==0)return(ValueBoard());
	if(Flag)alpha=-9999;
	else beta=9999;
	for(x=0;x<MASU_NUM;x++)
	{
		for(y=0;y<MASU_NUM;y++)
		{
			if(CanDropDown(x,y))
			{
				FlagForPut=TRUE;
				InitAndo(&ando,x,y);
				Reverse(&ando);
				DropDownStone(x,y);
				temp=Min_Max(!Flag,lv-1,TRUE,alpha,beta);
				ReReverse(ando);
				if(Flag)
				{
					if(temp>=alpha)
					{
						alpha=temp;
						vest_x=x;
						vest_y=y;
					}
					if(alpha>beta)return(alpha);
				}
				else
				{
					if(temp<=beta)
					{
						beta=temp;
						vest_x=x;
						vest_y=y;
					}
					if(alpha>beta)return(beta);
				}
			}
		}
	}
	if(FlagForPut)
	{
		if(lv==m_SearchLv)return(vest_x+vest_y*MASU_NUM);
		else if(Flag)return(alpha);
		else return(beta);
	}
	else
	if(!Put)return(ValueBoard());
	else
	{
		m_FlagForWhite=!m_FlagForWhite;
		temp=Min_Max(!Flag,lv-1,FALSE,alpha,beta);
		m_FlagForWhite=!m_FlagForWhite;
		return(temp);
	}
}

void ComputerAI()
{
	int x,y;
	Ando ando;
	if(m_PutNumber>=FINISH_NUM)
		y=Min_Max(
		TRUE,
		m_SearchLv=12,
		TRUE,
		-9999,
		9999);
	else y=Min_Max(
		TRUE,
		m_SearchLv=SEARCH_LV,
		TRUE,
		-9999,
		9999);
	if(0>y||y>=MASU_NUM*MASU_NUM)
	{
		m_FlagForWhite=!m_FlagForWhite;
		return;
	}
	x=y%MASU_NUM;
	y=y/MASU_NUM;
	InitAndo(&ando,x,y);
	Reverse(&ando);
	DropDownStone(x,y);
	m_PutNumber++;
	HDC hdc=GetDC(hWnd);
	DrawBoard(hdc);
	ReleaseDC(hWnd,hdc);
	for(x=0;x<MASU_NUM*MASU_NUM;x++)
	{
		if(m_PutNumber==60)break;
		if(CanDropDown(x%MASU_NUM,x/MASU_NUM))break;
		if(x==MASU_NUM*MASU_NUM-1)
		{
			m_FlagForWhite=!m_FlagForWhite;
			ComputerAI();
		}
	}
}

DWORD WINAPI MyThread(LPVOID WinObjPtr)
{
	while(g_ThreadFlag);
	EnableMenuItem(
		GetMenu(hWnd),
		ID_START_BLACK,
		MF_GRAYED);
	EnableMenuItem(
		GetMenu(hWnd),
		ID_START_WHITE,
		MF_GRAYED);
	DrawMenuBar(hWnd);
	g_ThreadFlag=TRUE;
	Sleep(250);ComputerAI();
	g_ThreadFlag=FALSE;
	EnableMenuItem(
		GetMenu(hWnd),
		ID_START_BLACK,
		MF_ENABLED);
	EnableMenuItem(
		GetMenu(hWnd),
		ID_START_WHITE,
		MF_ENABLED);
	DrawMenuBar(hWnd);
	return(0);
}

LRESULT CALLBACK WndProc(
						 HWND hWnd,
						 UINT msg,
						 WPARAM wParam,
						 LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	static DWORD d;
	switch(msg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_START_BLACK:
			m_FlagInGame=TRUE;
			m_FlagForPlayer=FALSE;
			InitBoard();
			hdc=GetDC(hWnd);
			DrawBoard(hdc);
			ReleaseDC(hWnd,hdc);
			break;
		case ID_START_WHITE:
			m_FlagInGame=TRUE;
			m_FlagForPlayer=TRUE;
			InitBoard();
			hdc=GetDC(hWnd);
			DrawBoard(hdc);
			ReleaseDC(hWnd,hdc);
			hThread1=CreateThread(
				NULL,
				0,
				MyThread,
				(LPVOID)NULL,
				0,
				&d);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		{
			if(!m_FlagInGame)return 0;
			else if(g_ThreadFlag)return 0;
			int x,y;
			x=LOWORD(lParam)/MASU_SIZE;
			y=HIWORD(lParam)/MASU_SIZE;
			if(CanDropDown(x,y))
			{
				Ando ando;
				InitAndo(&ando,x,y);
				Reverse(&ando);
				DropDownStone(x,y);
				m_PutNumber++;
				hdc=GetDC(hWnd);
				DrawBoard(hdc);
				ReleaseDC(hWnd,hdc);
				if(m_FlagInGame)
				{
					hThread1=CreateThread(
						NULL,
						0,
						MyThread,
						(LPVOID)NULL,
						0,
						&d);
				}
			}
		}
		break;
	case WM_CREATE:
		m_FlagInGame=FALSE;
		break;
	case WM_PAINT:
		hdc=BeginPaint(hWnd,&ps);
		DrawBoard(hdc);
		EndPaint(hWnd,&ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd,msg,wParam,lParam);
	}
	return (0L);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass={
		0,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_3DFACE+1),
		(LPCTSTR)IDR_MENU1,
		szClassName
	};
	RegisterClass(&wndclass);
	RECT rect={0,0,BOARD_SIZE,BOARD_SIZE};
	AdjustWindowRect(
		&rect,
		WS_OVERLAPPED|
		WS_CAPTION|
		WS_SYSMENU,TRUE
		);
	hWnd=CreateWindow(
		szClassName,
		TEXT("リバーシ"),
		WS_OVERLAPPED|
		WS_CAPTION|
		WS_SYSMENU,
		CW_USEDEFAULT,
		0,
		rect.right-rect.left,
		rect.bottom-rect.top,
		0,
		0,
		hInstance,
		0);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
