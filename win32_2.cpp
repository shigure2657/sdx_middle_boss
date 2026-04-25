#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

void ascii_to_hex(char* byte, char* buffer, int n);
void hex_to_ascii(char* str, char* out, int n);
int how_many_bytes_remain();
int insert_mboss();

struct middle_boss_list {
    LPCTSTR name;
    int id;
};

FILE *fp;
long file_size;
int file_open_flag = 0;
int map_size = 0;
int write_size = 0;
int middle_bytes = 0;   // 追加前の中ボスデータのバイト数
char middle_boss[] = "110001000000000001004374016C00FFFF";
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
    memset(dummy, 'F', map_size*2 + 2);
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
     // ファイル先頭から0x300000バイト先に移動する
    if(fseek(fp, 0x300000, SEEK_SET))
    {
         MessageBox(NULL, "fseek エラー","情報", MB_OK);
        return 0;  // 異常終了
    }

    fread(buffer, 0xFFFFF, 1, fp);
    ascii_to_hex(buffer,buffer2,file_size);
    address = strstr(buffer2,dummy);
    
	if ( address == NULL ) {
        MessageBox(NULL, "ダミーデータを見つけられませんでした。","情報", MB_OK);
		return 0;
	}

    //MessageBox(NULL, offset, "オフセット", MB_OK);
    return (address - &buffer2[0])/2;
}

void set_dummy_bytes(long address)
{
     char* buffer1 = ( char* )malloc(map_size*2);
     char* buffer2 = ( char* )malloc(map_size + 1);
	 memset(buffer1 , 'F', map_size*2);
     hex_to_ascii(buffer1,buffer2,map_size);
     fseek(fp,address, SEEK_SET);
     fwrite(buffer2,map_size, 1,fp);
	 free(buffer1);
	 free(buffer2);
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
	int i = 116;  // 移動データまでは（おそらく）58バイト固定
    char byte[32];
	char byte2[32];
    char* buffer = ( char* )malloc(map_size*2+1);

    ascii_to_hex(map_data,buffer,map_size);    // asciiから16進数へ変換
    buffer[map_size] = '\0';
    byte2[0] = buffer[i + 2];                  //上位バイトと下位バイトが逆順になっている
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
    free(buffer);
	return (i + (2*n));
}

// 追加前の中ボスデータのサイズ（なにもはいってないと、04 00 FF FFで４バイトとなる）
int size_of_middle_boss(int pos, char* map_data)
{
    int position = pos;
	char middle_bytes[4];
	char out[4];
	char* buffer = ( char* )malloc(map_size*2+1);
	ascii_to_hex(map_data,buffer,map_size);
	strncpy(middle_bytes, buffer + position, 4);
    out[0] = middle_bytes[2];
    out[1] = middle_bytes[3];
	out[2] = middle_bytes[0];
	out[3] = middle_bytes[1];
	out[4] = '\0';
    int byte = atoi(out);
	free(buffer);
	return byte;
}

char* create_map()
{
	long offset = 0x30244A;  // 春風とともに
	long insert_pos = 10;
	
    
    long new_size = map_size + strlen(middle_boss);
    char* buffer = ( char* )malloc(map_size+1);
	char* buffer2 = ( char* )malloc(map_size*2+1);  // 16進数に変換するとasciiコードよりも２倍の文字数になる為 +1はnull文字
    char* new_buffer = (char* ) malloc(new_size + 1);
    char* new_buffer2 = (char* ) malloc(new_size*2 + 1);  

    fseek(fp,offset, SEEK_SET);							// マップデータの位置までファイルポインタを移動
    fread(buffer, 1, map_size, fp);						// map_sizeの大きさのマップデータをbufferへ読み込み
    insert_pos = seek_middle(buffer);					// buffer(マップデータ)内の中ボスデータを追加するアドレスを調べる
    middle_bytes = size_of_middle_boss(insert_pos,buffer);
    memcpy(new_buffer, buffer, insert_pos);				// insert_pos(bufferの最初の位置から１６進数で何バイト中ボスを入れる位置まであるか）分だけbufferからnew_bufferへデータをコピー
	ascii_to_hex(new_buffer,new_buffer2,new_size);		// new_bufferを16進数へ変換して new_buffer2へ入れる
	memcpy(new_buffer2 + insert_pos, middle_boss, strlen(middle_boss));			// insert_posの位置へmiddle_boss(中ボスデータ)を、strlenの文字分だけコピー
    ascii_to_hex(buffer,buffer2,new_size);										// buffer (asciiのままのマップデータ)をbuffer2へasciiへ変換して入れる
	memcpy(new_buffer2 + insert_pos + strlen(middle_boss), buffer2 + insert_pos + 2*middle_bytes ,map_size*2 - insert_pos - 2*middle_bytes);    //  insert_pos＋中ボスデータの位置に、追加する前のマップデータ+insert_pos + middle_bytesからの残りのマップデータを入れる
    new_buffer2[new_size*2 - strlen(middle_boss) - 2*middle_bytes] = '\0';	// middle_bossのサイズも２倍になってるので増えた分だけ(strlen(middle_boss)分だけ減らす
    write_size = new_size*2 - strlen(middle_boss) - 2*middle_bytes;
    free(buffer);
	free(buffer2);
	return new_buffer2;
}

// マップデータのアドレスを変更する
void change_index(int offset)
{
	char index[5];
	char index_out[8];
	char dummy_offset[8];
	char dummy_offset_out[5];
    long index_offset = 0x3F009E;
    fseek(fp,index_offset, SEEK_SET);
	fread(index, 1, 4, fp);
	index[5] = '\0';
    ascii_to_hex(index,index_out,4);
    index_out[8] = '\0';
	 itoa(offset,dummy_offset,16);
	 index_out[0] = dummy_offset[4];
	 index_out[1] = dummy_offset[5];
	 index_out[2] = dummy_offset[2];
	 index_out[3] = dummy_offset[3];
     index_out[5] = dummy_offset[1];
	hex_to_ascii(index_out,dummy_offset_out,8);
    dummy_offset[6] = '\0';
	MessageBox(NULL, dummy_offset_out, "情報", MB_OK);
    fseek(fp,index_offset, SEEK_SET);
	fwrite(dummy_offset_out,4,1,fp);
}

int insert_mboss()
{
     int offset = 0;
     char* write_address;
	 long map_address = 0x30244A;

    if(how_many_bytes_remain() > 17)
	{
       return 0;
	}
	else
	{
		// マープデータのサイズの大きさのダミーデータ(0xFFで構成されたデータ)を検索
       offset = dummy_space_search();
       char temp[]  ="";
        
	   if(offset == 0)
		   return 0;
       offset += 0x300000; // 0x300000以降のアドレスしかマップデータを入れられない？
	   offset++; // 1増やして1バイト上書きしないように
	   write_address = create_map();
       char* buffer = ( char* )malloc(map_size+strlen(middle_boss)/2);
	   hex_to_ascii(write_address,buffer,map_size+strlen(middle_boss) - middle_bytes);
      // MessageBox(NULL, write_address, "情報", MB_OK);
       fseek(fp,offset, SEEK_SET);
	   fwrite(buffer,map_size + (strlen(middle_boss)/2) - middle_bytes,1,fp);
	   set_dummy_bytes(map_address);
	   change_index(offset);
	   return 1;
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
	char id[4];
	int size_x, size_y;
	int i = 0;
	int l = 0;
    int value_x, value_y;
    int swapped_x,swapped_y;

	struct middle_boss_list list[7] = {{"アイアンマム",41},{"ジュキッド", 42},{"ポピーブロスSr", 43},{"コックカワサキ",44},{"Mr.フロスティ",45},
	{"ボンカース",46}, {"バグジー",47}};

    switch (message)
    {
    case WM_INITDIALOG:
        for ( i = 0; i < sizeof(list) / sizeof(list[0]); i++)
		{
		    SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)list[i].name);
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
			if(insert_mboss())
            {
                 MessageBox(NULL, "中ボスデータを追加しました。", "情報", MB_OK);
			}
			else
			{
                 MessageBox(NULL, "中ボスデータの追加に失敗しました。", "エラー", MB_ICONWARNING | MB_OK);
			}
			//change_index();
		}
        if( LOWORD(wParam) == IDC_BUTTON3)
		{
            GetDlgItemTextA(hDlg,IDC_EDIT1,x_axis,4);
            GetDlgItemTextA(hDlg,IDC_EDIT2,y_axis,4);
			int index = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);
			itoa(list[index].id,id,10);
			//MessageBox(NULL, id, "中ボスid", MB_OK);
			
            x = strtol(x_axis, &endp, 10);
            y = strtol(y_axis, &endp, 10);
            sprintf(x_hex,"%x",x);
            sprintf(y_hex,"%x",y);
			//MessageBox(NULL, x_axis, "y座標", MB_OK);
            value_x = strtoul(x_hex, NULL, 16);
            value_y = strtoul(y_hex, NULL, 16);

     
            swapped_x = ((value_x & 0xFF) << 8) | ((value_x >> 8) & 0xFF);
			swapped_y = ((value_y & 0xFF) << 8) | ((value_y >> 8) & 0xFF);
			sprintf(out_x, "%04X", swapped_x);
			sprintf(out_y, "%04X", swapped_y);
			MessageBox(NULL, out_x, "x座標", MB_OK);
            sprintf(middle_boss,"11000100000000000100%s%s%sFFFF",id,out_x,out_y);
            
		}
		if (LOWORD(wParam) == IDC_BUTTON1) 
		{
		   if( file_open())
               SetDlgItemTextA(hDlg, IDC_STATIC2, rom_name);
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