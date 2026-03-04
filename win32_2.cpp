#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

void ascii_to_hex(char* byte, char* buffer, int n);
int how_many_bytes_remain();
void insert_mboss();

LPCTSTR items[] = {
	"dog",
	"cat",
	"rabbit",
	"fox",
	"monkey",
	"hamster"
};

FILE *fp;
long file_size;
int file_open_flag = 0;
int map_size = 0;
int write_size = 0;
char middle_boss[] = "110001000000000001004374016C00FFFF";
//LPSTR x_axis = TEXT("0");
char x_axis[] = "0";
char y_axis[] = "0";

int check_title()
{
	char byte[32];
	char buffer[64];
    long offset = 0x7FC0; 
	fseek(fp,offset, SEEK_SET);
    fread(byte,17,1,fp);
	byte[17] = '\0';

	if(strcmp(byte,"KIRBY SUPER DELUX") == 0) 
	{
        MessageBox(NULL, byte, "情報", MB_OK);
		return 1;
	}
	else
	{
        MessageBox(NULL, "適切なROMファイルではありません", "情報", MB_OK);
		return 0;
	} 
}

int file_open()
{
	unsigned char buf;
    //ファイルパスを格納する変数
	//0で初期化しておく
	char filePath[MAX_PATH] = { 0 };

	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);		//構造体のサイズ
	ofn.hwndOwner = NULL;						//オーナーウィンドウのハンドル
	ofn.lpstrFilter = "全てのファイル (*.*)\0*.*\0"; //拡張子フィルター
	ofn.nFilterIndex = 0;			//フィルターの初期値
	ofn.lpstrFile = filePath;		//選択したファイルパスを受け取るバッファ
	ofn.nMaxFile = MAX_PATH;		//↑のバッファサイズ

	if (GetOpenFileName(&ofn)) {
		MessageBox(NULL, filePath, "情報", MB_OK);
		fp = fopen(filePath,"rb+");
		
		if(check_title() != 1){
			fclose(fp);
			return 0;
		}
        file_open_flag = 1;
	}
	else {
		MessageBox(NULL, "キャンセルされました", "情報", MB_OK);
		return 0;
	}
    // ファイルサイズを取得
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
	return 1;
}

long dummy_space_search()
{
	int c = 0;
	char dummy[2048];
	char *address;
	char offset[] = "";
    memset(dummy, '\0', map_size*2 + 1);
    memset(dummy, 'F', map_size*2 + 1);
	dummy[map_size*2+1]= '\0';
    // メモリ確保して全データ読み込み
     char* buffer = ( char* )malloc(file_size);
     char* buffer2 = ( char* )malloc(file_size*2);
    if (buffer == NULL) {
        MessageBox(NULL, "メモリ確保に失敗しました", "情報", MB_OK);
        fclose(fp);
        return 0 ;
    }
	fseek(fp,0, SEEK_SET);
    fread(buffer, 1, file_size, fp);
    ascii_to_hex(buffer,buffer2,file_size);
    address = strstr(buffer2,dummy);
	//sprintf(offset,"%X",(address - &buffer2[0])/2);
    //MessageBox(NULL, offset, "情報", MB_OK);
    return (address - &buffer2[0])/2;
}

void set_dummy_bytes()
{
    
}

void ascii_to_hex(char* byte, char* buffer, int n)
{
	int j = 0;
    for (int i = 0; i < n; i++) {
        char temp[2];  // 2桁 + null文字
        sprintf(temp, "%02X", (unsigned char)byte[i]);  // ASCIIコードを16進数（大文字）で文字列化
		buffer[j] = temp[0];
		buffer[j+1] = temp[1];
        j += 2;
    }
}

void hex_to_ascii(char* str, char* out, int n)
{
    for (int i = 0; i < n; i++) {
        char temp[3] = { str[i * 2], str[i * 2 + 1], '\0' };
        int ascii_code = (int)strtol(temp, NULL, 16);  
        out[i] = (char)ascii_code;
    }
}

long seek_middle(char *map_data)
{
	int n = 0;
	int i = 116;
    char byte[32];
	char byte2[32];
    char* buffer = ( char* )malloc(map_size*2+1);

    ascii_to_hex(map_data,buffer,map_size);
    buffer[map_size] = '\0';
    byte2[0] = buffer[i + 2];
	byte2[1] = buffer[i + 3];
	byte2[2] = buffer[i];
	byte2[3] = buffer[i + 1];
	byte2[4] = '\0';
    n = strtol(byte2, NULL, 16);
	byte2[0] = buffer[i + 2 + (2*n)];
	byte2[1] = buffer[i + 3 + (2*n)];
	byte2[2] = buffer[i + (2*n)];
	byte2[3] = buffer[i + 1 + (2*n)];
	byte2[4] = '\0';
	i = i + (2*n);
	n = strtol(byte2, NULL, 16);
    byte2[0] = buffer[i + 2 + (2*n)];
	byte2[1] = buffer[i + 3 + (2*n)];
	byte2[2] = buffer[i + (2*n)];
	byte2[3] = buffer[i + 1 + (2*n)];
	byte2[4] = '\0';
    MessageBox(NULL, byte2, "情報", MB_OK);

	return (i + (2*n));
}
char* create_map()
{
	long offset = 0x30244A;  // 春風とともに
	long insert_pos = 10;
    
    long new_size = map_size + strlen(middle_boss);
    char* buffer = ( char* )malloc(map_size+1);
	char* buffer2 = ( char* )malloc(map_size*2+1);
    char* new_buffer = (char* ) malloc(new_size);
    char* new_buffer2 = (char* ) malloc(new_size*2);

    fseek(fp,offset, SEEK_SET);
    fread(buffer, 1, map_size, fp);
    insert_pos = seek_middle(buffer);
    memcpy(new_buffer, buffer, insert_pos); 
	ascii_to_hex(new_buffer,new_buffer2,new_size);
	memcpy(new_buffer2 + insert_pos, middle_boss, strlen(middle_boss));
    ascii_to_hex(buffer,buffer2,new_size);
	memcpy(new_buffer2 + insert_pos + strlen(middle_boss), buffer2 + insert_pos,map_size*2 - insert_pos);
    new_buffer2[new_size*2 - strlen(middle_boss)] = '\0';
    write_size = new_size*2 - strlen(middle_boss);

	return new_buffer2;
}

void change_index()
{
    
}

void insert_mboss()
{
     long offset = 0;
     char* write_address;

    if(how_many_bytes_remain() > 17)
	{
       
	}
	else
	{
       offset = dummy_space_search();
       char temp[]  ="";

	   if(offset == 0)
		   return;

	   write_address = create_map();
       char* buffer = ( char* )malloc(map_size/2+1);
	   hex_to_ascii(write_address,buffer,map_size);
       MessageBox(NULL, write_address, "情報", MB_OK);
       fseek(fp,offset, SEEK_SET);
	   fwrite(buffer,map_size,1,fp);
	}

}

void load_map()
{
    
	int i = 0;
	int j = 0;
	int first = 0; 
	int later = 0;
	char print_size[256];
	long temp = 0;
	int found_flag = 0;
	char byte[32];
	char buffer[64] = "";
    long offset = 0x30244A;  // 春風とともに

	if(file_open_flag == 0) 
	{
          MessageBox(NULL, "ROMを開いてません", "情報", MB_ICONWARNING | MB_OK);
		  return;
	}

	fseek(fp,offset, SEEK_SET);
    while(i <  0xFFFF)
	{
		fread(byte,2,1,fp);
        byte[2] = '\0';
        ascii_to_hex(byte,buffer,2);
        
        buffer[4] = '\0';
		if(buffer[0] == 'E'  && buffer[1] == 'F' && buffer[2] == '3' && buffer[3] == 'D'){
	        MessageBox(NULL, "found", "情報", MB_OK);
            first = ftell(fp) - offset;
			temp = ftell(fp);
            while(j < 0xFFFF) {
                 fread(byte,2,1,fp);
                 ascii_to_hex(byte,buffer,2);
                 buffer[4] = '\0';
				 if(buffer[0] == 'F' && buffer[1] == 'F' && buffer[2] == 'F') {
                      later = ftell(fp) - temp;
					  found_flag = 1;
					  break;
				 }   
				 j++;
			}
			break;
		}
        i++;
	}

    if (found_flag == 1){
        map_size = first + later;
		sprintf(print_size,"%x",map_size);
        MessageBox(NULL, print_size, "情報", MB_OK);
	}
}

int how_many_bytes_remain()
{
    long offset = 0x30244A;  // 春風とともに
	int count = 0;
	char dum[4];
	char buffer[8] = "";
    fseek(fp,offset + map_size, SEEK_SET);
    fread(dum,1,1,fp);
    ascii_to_hex(dum,buffer,1);
	while (buffer[0] == 'F' && buffer[1] == 'F') {
        count++;
        fread(dum,1,1,fp);
        ascii_to_hex(dum,buffer,1);
	}
	char temp[32];
	sprintf(temp,"%d", count);
    //MessageBox(NULL, temp, "情報", MB_OK);
	return count;
}

// ダイアログのプロシージャ
BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND combo;
    combo = GetDlgItem(hDlg, IDC_COMBO2);
    static CONST LPSTR rom_name = TEXT("KIRBY SUPER DELUX");
    char *endp;
	int x = 0;
	int y = 0;
	char x_hex[4];
	char y_hex[4];
	char out_x[5];
	char out_y[5];

	int i;
    switch (message)
    {
    case WM_INITDIALOG:
        for ( i = 0; i < sizeof(items) / sizeof(items[0]); i++)
		{
		    SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)items[i]);
		}
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_OK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
			if(file_open_flag == 1)
                fclose(fp);
            return (INT_PTR)TRUE;
        }
		if( LOWORD(wParam) == IDC_BUTTON2)
		{
            load_map();
			insert_mboss();
		}
        if( LOWORD(wParam) == IDC_BUTTON3)
		{
            GetDlgItemTextA(hDlg,IDC_EDIT1,x_axis,4);
            GetDlgItemTextA(hDlg,IDC_EDIT2,y_axis,4);
            x = strtol(x_axis, &endp, 10);
            y = strtol(y_axis, &endp, 10);
            sprintf(x_hex,"%x",x);
            sprintf(y_hex,"%x",y);
            
			for (int i = 0; i < 4; i++) {
                if(x_hex[i] != '\0')
					out_x[3 - i] = x_hex[i];
				else {
					while(i < 4) {
                        out_x[3 - i] = '0';
						i++;
					}
				}
			}
			i = 0;
            for (i = 0; i < 4; i++) {
			    if(y_hex[i] != '\0')
					out_y[3 - i] = y_hex[i];
				else {
					while(i < 4) {
                        out_y[3 - i] = '0';
						i++;
					}
				}
			}
            out_x[4] = '\0';
            out_y[4] = '\0';
            sprintf(middle_boss,"11000100000000000100%s%s00FFFF",out_x,out_y);
            MessageBox(NULL, middle_boss, "情報", MB_OK);
		}
		switch(LOWORD(wParam)) {
		    case IDC_BUTTON1:
		   if( file_open())
                SetDlgItemTextA(hDlg, IDC_STATIC2, rom_name);
			//SetDlgItemTextA(hDlg,IDC_STATIC,"SAMPLE");
			break;
			case IDC_BUTTON2:
           // MessageBox(NULL,"!", "情報", MB_OK);
			//load_map();
            //dummy_space_search();
            //how_many_bytes_remain();
            break;
		}
        break;
    }
    return (INT_PTR)FALSE;
}

// WinMain: エントリポイント
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, DialogProc);
    return 0;
}