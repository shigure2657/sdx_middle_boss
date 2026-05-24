#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#define MAP_NUMBERS 318

void ascii_to_hex(char* byte, char* buffer, int n);
void hex_to_ascii(char* str, char* out, int n);
int how_many_bytes_remain(HWND hDlg);
int insert_mboss();
int map_address(HWND hDlg);
int get_map_address_from_combo(HWND hDlg);

struct middle_boss_list {
    LPCTSTR name;
    int id;
};

struct map_data {
    LPCSTR name;
	int address;
};

struct after_defeat {
    LPCSTR name;
	int defeat_id;
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
char x_axis2[] = "0";
char y_axis2[] = "0";

int check_title()
{
	char byte[32];
    long offset = 0x7FC0; 

	fseek(fp,offset, SEEK_SET);
    fread(byte,17,1,fp);
	byte[17] = '\0';

    if(fseek(fp, 0, SEEK_END))
    {
         MessageBox(NULL, "fseek エラー","情報", MB_OK);
        return 0;  // 異常終了
    }
    
	if(strcmp(byte,"KIRBY SUPER DELUX") == 0 && ftell(fp) == 0x400000) 
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

long dummy_space_search(HWND hDlg)
{
	int c = 0;
	char dummy[2048];
	char *address;
	char offset[] = "";

	int middle_size = 0x11;

	// 撃破後の処理のチェックがある場合
	if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK , 0 , 0))
	{
         middle_size = 0x16;
	}

    memset(dummy, '\0', map_size*2 + 1 );
    memset(dummy, 'F', map_size*2 + 2 );
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
	 //free(buffer1);
	 //free(buffer2);
}

// nはasciiのデータ長
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

// 16進数からASCIIへ変換 nはasciiのデータ長
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
    MessageBox(NULL, byte2, "seek_middle", MB_OK);
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
    //int byte = atoi(out);
	MessageBox(NULL, out, "size of middle boss", MB_OK);
	int byte = (int)strtol(out, NULL, 16);
	free(buffer);
	return byte;
}

char* create_map(HWND hDlg)
{
	int i = 0;
    int j = 0;
	int size = 0;  // 追加後の中ボスデータのサイズ
	long offset = 0;
	long insert_pos = 10;
	char size_middle[4];
	char addr[8];
	char middle_extract[128];    // 中ボスが存在する場合、バイト数を表すデータを取り除く
	
	HWND combo = GetDlgItem(hDlg, IDC_COMBO1);
    int index = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);
	SendMessage(combo, CB_GETLBTEXT, index, (LPARAM)addr);  // コンボボックスから文字列を取得
    offset = strtol(addr, NULL, 16); 
    
    long new_size = map_size + strlen(middle_boss);
    char* buffer = ( char* )malloc(map_size+1);
	char* buffer2 = ( char* )malloc(map_size*2+1);  // 16進数に変換するとasciiコードよりも２倍の文字数になる為 +1はnull文字
    char* new_buffer = (char* ) malloc(new_size + 1);
    char* new_buffer2 = (char* ) malloc(new_size*2 + 1);  
	

    fseek(fp,offset, SEEK_SET);								// マップデータの位置までファイルポインタを移動
    fread(buffer, 1, map_size, fp);							// map_sizeの大きさのマップデータをbufferへ読み込み
    insert_pos = seek_middle(buffer);						// buffer(マップデータ)内の中ボスデータを追加するアドレスを調べる
    middle_bytes = size_of_middle_boss(insert_pos,buffer);  // 中ボスデータのサイズを取得
    int if_middle_add = 2*middle_bytes - 4;
	// もし中ボスデータがひとつ以上入っているなら
	// 最初の 11 00 を取り除く
	if (middle_bytes > 4)
	{
		int i = strlen(middle_boss) - 4;  // 文字数はバイト数 - 4
		//char s[4];
		//itoa((i+4)/2-4,s,16);
		//MessageBox(NULL, s, "s", MB_OK);
        strncpy(middle_extract, middle_boss + 4, i); // 30は0x11 * 2 - 4
		middle_extract[i] = '\0';
		strcpy(middle_boss, middle_extract);
		size = middle_bytes + (i+4)/2-4; // 0xdは 0x11 - 0x04 
	}
    else
	{
        if_middle_add = 0;
		size = (i+4)/2; // sizeは追加する中ボスデータのバイト数
	}

    memcpy(new_buffer, buffer, insert_pos);					// insert_pos(bufferの最初の位置から１６進数で何バイト中ボスを入れる位置まであるか）分だけbufferからnew_bufferへデータをコピー
	ascii_to_hex(new_buffer,new_buffer2,new_size);			// new_bufferを16進数へ変換して new_buffer2へ入れる
				// insert_pos + 元の中ボスデータサイズの位置 へmiddle_boss(中ボスデータ)を、strlenの文字分だけコピー、追加
	memcpy(new_buffer2 + insert_pos + if_middle_add , middle_boss, strlen(middle_boss)); // 既存の中ボスデータを上書きしないように修正
    ascii_to_hex(buffer,buffer2,new_size);										// buffer (asciiのままのマップデータ)をbuffer2へasciiへ変換して入れる
						// insert_pos + 元の中ボスデータサイズの位置 + 中ボスデータの位置に、追加する前のマップデータ+insert_pos + middle_bytesからの残りのマップデータを入れる
	memcpy(new_buffer2 + insert_pos + if_middle_add  + strlen(middle_boss), buffer2 + insert_pos + 2*middle_bytes ,map_size*2 - insert_pos - 2*middle_bytes );	
    
	if (middle_bytes > 4)
	{
	    new_buffer2[new_size*2 - strlen(middle_boss ) ] = '\0';
	    write_size = new_size*2 - strlen(middle_boss );
	}
    else
    {
	    new_buffer2[new_size*2 - strlen(middle_boss) - 2*middle_bytes] = '\0';		// middle_bossのサイズも２倍になってるので増えた分だけ(strlen(middle_boss)分だけ減らす
	    write_size = new_size*2 - strlen(middle_boss) - 2*middle_bytes;          // 元の中ボスデータを上書きしたのでそれも引く( - 2*middle_bytes)
	}
    
	itoa(size, size_middle, 16);
	MessageBox(NULL, size_middle, "size_middle", MB_OK);
     new_buffer2[insert_pos] = size_middle[0];   // 中ボスデータの最初のバイト数の変更
	 new_buffer2[insert_pos + 1] = size_middle[1];
    //free(buffer);
	//free(buffer2);
	return new_buffer2;
}

// マップデータのアドレスを変更する
void change_index(int offset, HWND hDlg)
{
	char index[5];
	char index_out[8];
	char dummy_offset[8];
	char dummy_offset_out[5];
	int combo_index = (int)SendMessage( GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0);
    long index_offset = 0x3F009E + 4*combo_index;

    fseek(fp,index_offset, SEEK_SET);
	fread(index, 1, 4, fp);
	index[5] = '\0';

	// index_outは16進数なので2倍の長さ
    ascii_to_hex(index,index_out,4);
    
    index_out[8] = '\0';
	MessageBox(NULL, index_out, "index_out", MB_OK);
	 itoa(offset,dummy_offset,16);
	 index_out[0] = dummy_offset[4];
	 index_out[1] = dummy_offset[5];
	 index_out[2] = dummy_offset[2];
	 index_out[3] = dummy_offset[3];
     index_out[5] = dummy_offset[1];
	hex_to_ascii(index_out,dummy_offset_out,8);
    dummy_offset[6] = '\0';
	MessageBox(NULL, dummy_offset_out, "dummy_offset_out", MB_OK);
    fseek(fp,index_offset, SEEK_SET);
	fwrite(dummy_offset_out,4,1,fp);
}

// マップデータのアドレスを入手
int map_address(HWND hDlg)
{
	int i;
	char index[5];
	char index_out[8];
	char address[8];
    int index_offset_first = 0x3F009E;
	HWND combo = GetDlgItem(hDlg, IDC_COMBO1);
    //struct map_data map[MAP_NUMBERS];
    SendMessage(combo, CB_RESETCONTENT, 0, (LPARAM)address);

    for (i = 0; i < MAP_NUMBERS; i++)
	{
        fseek(fp,0x3F009E + 4*i, SEEK_SET);
		fread(index, 1, 4, fp);
	    index[5] = '\0';
	    ascii_to_hex(index,index_out,4);
        address[0] = '3';
	    address[1] = index_out[5];
	    address[2] = index_out[2];
        address[3] = index_out[3];
 	    address[4] = index_out[0];
	    address[5] = index_out[1];
	    address[6] = '\0';
		SendMessage(combo, CB_ADDSTRING, 0, (LPARAM)address);
	}
	return 0;
}

// 現在のコンボボックスで選択されたアドレスを取得
int get_map_address_from_combo(HWND hDlg)
{
	char addr[8];
    HWND combo = GetDlgItem(hDlg, IDC_COMBO1);
	int index = (int)SendMessage(combo, CB_GETCURSEL, 0, 0);
	SendMessage(combo, CB_GETLBTEXT, index, (LPARAM)addr);
	int map_addr = strtol(addr, NULL, 16); 
	return map_addr;
}

int insert_mboss(HWND hDlg)
{
     int offset = 0;
	 int dummy_flag = 0;
     char* write_address;
	 char addr[8];
	 //int map_addr = map_address(hDlg);
	 int map_addr = 0;
     write_address = create_map(hDlg);
    
	 map_addr = get_map_address_from_combo(hDlg);
	// 文字列をlong型の１６進数に変換

    // マップデータの後に何バイトダミーデータが残っているか
    if(how_many_bytes_remain(hDlg) > 0x11)
	{
	    offset = map_addr;
       //return 0;
	} 
	else
	{
		
		    // マープデータのサイズの大きさのダミーデータ(0xFFで構成されたデータ)を検索
        offset = dummy_space_search(hDlg);
        char temp[]  ="";
           
		// ダミーデータが見つからなかった
	    if(offset == 0)
		   return 0;
        dummy_flag = 1;
        offset += 0x300000; // 0x300000以降のアドレスしかマップデータを入れられない？
	    offset++;			// 1増やして1バイト上書きしないように
	}
	
    char* buffer = ( char* )malloc(map_size+strlen(middle_boss)/2);
	hex_to_ascii(write_address,buffer,map_size+strlen(middle_boss) );
    fseek(fp,offset, SEEK_SET);                       // ダミーデータの位置まで移動（すでに入っている場合は動かさない）

	if (middle_bytes > 4)
	    fwrite(buffer,map_size + (strlen(middle_boss)/2) - 2,1 ,fp); //　－２は　追加の場合は FF FF (2バイト分）減らす
	else
        fwrite(buffer,map_size + (strlen(middle_boss)/2) - 4,1 ,fp); // -4は　04 00 FF FF( 4バイト)分減らす
   
	// ダミーデータにマップデータを移した場合
	if (dummy_flag == 1) 
	{
	    set_dummy_bytes(map_addr);
	    change_index(offset, hDlg);
	}
	return 1;
}


int load_map(HWND hDlg)
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
    long offset = 0; 
	char addr[8];

    offset = get_map_address_from_combo(hDlg); // 文字列をlong型の１６進数に変換

   // MessageBox(NULL, addr, "CB_GETLBTEXT", MB_OK);
	if(file_open_flag == 0) 
	{
          MessageBox(NULL, "ROMを開いてません", "情報", MB_ICONWARNING | MB_OK);
		  return 1;
	}

	fseek(fp,offset, SEEK_SET);
    while(i <  0xFFFF)
	{
		fread(byte,2,1,fp);
        byte[2] = '\0';
        ascii_to_hex(byte,buffer,2);
         fseek(fp,-1, SEEK_CUR);
        buffer[4] = '\0';
		//if(strcmp(buffer,"EF3D")){ (strcmpだとなぜかうまくいかない）
		if(buffer[0] == 'E'  && buffer[1] == 'F' && buffer[2] == '3' && buffer[3] == 'D'){
	        MessageBox(NULL, "found", "情報", MB_OK);
            first = ftell(fp) - offset;
			temp = ftell(fp);
			
            while(j < 0xFFFF) {
                 fread(byte,2,1,fp);
                 ascii_to_hex(byte,buffer,2);
				 fseek(fp,-1, SEEK_CUR);
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
        MessageBox(NULL, print_size, "print_size", MB_OK);
	}

	return 0;
}

int how_many_bytes_remain(HWND hDlg)
{
	char addr[8];
	long offset = 0;
     // コンボボックスから文字列を取得
    offset =  get_map_address_from_combo(hDlg);   // 関数化
    
	int count = 0;
	char dum[4];
	char buffer[8] = "";
    fseek(fp,offset + map_size, SEEK_SET);
    fread(dum,1,1,fp);
    ascii_to_hex(dum,buffer,1);
	char sample[10];
	itoa(map_size,sample,16);
	while (buffer[0] == 'F' && buffer[1] == 'F') {
        count++;
        fread(dum,1,1,fp);
        ascii_to_hex(dum,buffer,1);
	}
	return count;
}

void add_boss_to_listbox(HWND hDlg)
{
    
	//fread middle boss ..
}

void listbox_number_of_boss(HWND hDlg)
{
    
}

// ダイアログのプロシージャ
BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND combo_number;
	HWND checkbox;

	combo_number = GetDlgItem(hDlg, IDC_COMBO5);
	checkbox = GetDlgItem(hDlg, IDC_CHECK1);

    static CONST LPSTR rom_name = TEXT("KIRBY SUPER DELUX");
    char *endp;
	int x = 0;
	int y = 0;
	int x2 = 0;
	int y2 = 0;
	char x_hex[4], y_hex[4];
	char x_hex2[4], y_hex2[4];
	char out_x[5], out_y[5];  // 中ボスの座標
	char out_x2[5], out_y2[5]; // 撃破後のスプライトの座標
	char id[4];
	char defeat_id[2];
	int size_x, size_y;
	int i = 0;
	int l = 0;
    int value_x, value_y, value_x2, value_y2;
    int swapped_x,swapped_y,swapped_x2,swapped_y2;
	int index = 0;
	int zako = 0;
	char sprite_action[2], sprite[2];

	struct middle_boss_list list[7] = {{"アイアンマム",41},{"ジュキッド", 42},{"ポピーブロスSr", 43},{"コックカワサキ",44},{"Mr.フロスティ",45},
	{"ボンカース",46}, {"バグジー",47}};
	LPCSTR number[3] = {"1","2","3"};
	struct after_defeat defeat[3] = {{"ゲートスター",1},{"ワープスター",2},{"クリアスター",3}};

    switch (message)
    {
    case WM_INITDIALOG:
        for ( i = 0; i < sizeof(list) / sizeof(list[0]); i++)
		{
		    SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_ADDSTRING, 0, (LPARAM)list[i].name);
		}

		for ( i = 0; i < 3; i++)
		{
            SendMessage(GetDlgItem(hDlg, IDC_COMBO6), CB_ADDSTRING, 0, (LPARAM)defeat[i].name);
		}

        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_OK || LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == 40003)
        {
            EndDialog(hDlg, LOWORD(wParam));
			if(file_open_flag == 1)
                fclose(fp);
            return (INT_PTR)TRUE;
        }
		if( LOWORD(wParam) == IDC_BUTTON2 || 40004)
		{
			if (SendMessage( GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0) == -1)
			{
                MessageBox(NULL, "マップアドレスを追加してください。", "エラー", MB_ICONWARNING | MB_OK);
				break;
			}

            if(load_map(hDlg))
			    break;

			if(insert_mboss(hDlg))
            {
                 MessageBox(NULL, "中ボスデータを追加しました。", "情報", MB_OK);
			}
			else
			{
                 MessageBox(NULL, "中ボスデータの追加に失敗しました。", "エラー", MB_ICONWARNING | MB_OK);
			}

            map_address(hDlg);
		}
		if ( LOWORD(wParam) == IDC_CHECK1)
		{
            if (SendMessage(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT3), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT4), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO6), TRUE);
			}
			else
			{
                EnableWindow(GetDlgItem(hDlg, IDC_EDIT3), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_EDIT4), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_COMBO6), FALSE);
			}
		}
        if( LOWORD(wParam) == IDC_BUTTON3)
		{
            GetDlgItemTextA(hDlg,IDC_EDIT1,x_axis,4); // 1体目の中ボスのx座標 10進数
            GetDlgItemTextA(hDlg,IDC_EDIT2,y_axis,4); // 1体目の中ボスのy座標 10進数
			GetDlgItemTextA(hDlg,IDC_EDIT3,x_axis2,4); // 1体目の中ボスの撃破後の処理のx座標 10進数
            GetDlgItemTextA(hDlg,IDC_EDIT4,y_axis2,4); // 1体目の中ボスの撃破後の処理のy座標 10進数

			int index = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0); // コンボボックスに設定したマップアドレスの取得
			int index2 = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO6), CB_GETCURSEL, 0, 0); 
			itoa(list[index].id,id, 10);
			itoa(defeat[index2].defeat_id, defeat_id, 10);

			if (index == NULL)
                MessageBox(NULL, "error", "id", MB_OK);
             if(id == '\0')
			 {
                  MessageBox(NULL, "中ボスを選択してください。", "エラー", MB_ICONWARNING | MB_OK);
				  break;
			 }
			
			if (x_axis[0] == '\0' || y_axis[0] == '\0')
			{
                MessageBox(NULL, "座標を入力してください。", "エラー", MB_ICONWARNING | MB_OK);
			    break;
			}

			// int型へ変換
            x = strtol(x_axis, &endp, 10);
            y = strtol(y_axis, &endp, 10);
			x2 = strtol(x_axis2, &endp, 10);
            y2 = strtol(y_axis2, &endp, 10);

			// 16進数へ変換
            sprintf(x_hex,"%x",x);
            sprintf(y_hex,"%x",y);
			sprintf(x_hex2,"%x",x2);
            sprintf(y_hex2,"%x",y2);

			// int型へ変換
            value_x = strtoul(x_hex, NULL, 16);
            value_y = strtoul(y_hex, NULL, 16);
			value_x2 = strtoul(x_hex2, NULL, 16);
            value_y2 = strtoul(y_hex2, NULL, 16);

			// 0を詰める (FF → 00FF)
            swapped_x = ((value_x & 0xFF) << 8) | ((value_x >> 8) & 0xFF);
			swapped_y = ((value_y & 0xFF) << 8) | ((value_y >> 8) & 0xFF);
			swapped_x2 = ((value_x2 & 0xFF) << 8) | ((value_x2 >> 8) & 0xFF);
			swapped_y2 = ((value_y2 & 0xFF) << 8) | ((value_y2 >> 8) & 0xFF);

			sprintf(out_x, "%04X", swapped_x);
			sprintf(out_y, "%04X", swapped_y);
			sprintf(out_x2, "%04X", swapped_x2);
			sprintf(out_y2, "%04X", swapped_y2);

			// 雑魚敵を残すにチェックが入っている場合
			if(BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK2) , BM_GETCHECK , 0 , 0))
			{
                 zako = 1;
			}
            
			int number = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO6), CB_GETCURSEL, 0, 0);
            number++;    
			 // ワープスター
			if (number == 2)
			{
                strcpy(sprite_action, "00"); // 行動パターンは00
				strcpy(sprite, "84");    // 撃破後の処理をワープスターにしたときのスプライト番号はなぜか84
                MessageBox(NULL, sprite_action, "sprite_action", MB_OK);
			} // クリアスター
			else if (number == 3)
			{
                strcpy(sprite_action, "00"); // 行動パターンは00
                strcpy(sprite, "73");     // スプライト番号は73
			}
            
			// 撃破後の処理にチェックが入っている場合
			if(BST_CHECKED == SendMessage(checkbox , BM_GETCHECK , 0 , 0))
			{
				// 撃破後の処理がゲートスターの場合
				if ( number == 1)
				{
                    sprintf(middle_boss,"13000%d000000000%d%s%s0100%s%s%sFFFF",zako ,number , sprite_action, sprite, id ,out_x, out_y);
				}// それ以外
				else 
				{
                    sprintf(middle_boss,"17000%d000000000%d%s%s%s%s0100%s%s%sFFFF",zako ,number , sprite_action, sprite, out_x2 ,out_y2 ,id ,out_x, out_y);
				}
			}
            else
			{
                 sprintf(middle_boss,"11000%d00000000000100%s%s%sFFFF",zako ,id,out_x,out_y);
			}
            
		}
		if (LOWORD(wParam) == IDC_BUTTON1 || LOWORD(wParam) == 40002) 
		{
		   if( file_open())
		   {
               SetDlgItemTextA(hDlg, IDC_STATIC2, rom_name);
		       map_address(hDlg);
		   }
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