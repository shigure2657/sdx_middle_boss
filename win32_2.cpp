#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"

#define MAP_NUMBERS 318
#define MAX_BOSS_NUM 256

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

typedef struct   {
    char action[3];  // 中ボスの行動パターン
	char sprite[3]; // 中ボスのスプライト番号 (41-47)
    char boss_x[6]; // 中ボスのx座標
	char boss_y[6]; // 中ボスのy座標
} boss_data ;

typedef struct  {
    char zako[3];     // 雑魚敵を残すか否か
    char defeat[3];   // 撃破後の処理の有無
	char sprite_action[3]; // 撃破後の処理のスプライトの行動パターン
	char sprite[3]; // 撃破後のスプライト番号
    char defeat_x[6];
	char defeat_y[6];
    char number[3];  // 出現数
	boss_data boss[MAX_BOSS_NUM]; // 出現数分だけ
} DATA ;

void ascii_to_hex(char* byte, char* buffer, int n);
void hex_to_ascii(char* str, char* out, int n);
int how_many_bytes_remain(HWND hDlg);
int insert_mboss();
int map_address(HWND hDlg);
int get_map_address_from_combo(HWND hDlg);
void listbox_number_of_boss(HWND hDlg, DATA* data, int size);
void add_to_list(HWND hDlg);

//struct middle_data data[32];
struct middle_boss_list list[7] = {{"アイアンマム",41},{"ジュキッド", 42},{"ポピーブロスSr", 43},{"コックカワサキ",44},{"Mr.フロスティ",45},
	{"ボンカース",46}, {"バグジー",47}};
struct after_defeat defeat[3] = {{"ゲートスター",1},{"ワープスター",2},{"スプライト",3}};
DATA data[32];

FILE *fp;
long file_size;
int file_open_flag = 0;
int rom_flag = 0;
int map_size = 0;
int write_size = 0;
int middle_bytes = 0;   // 中ボスデータのバイト数
int middle_before = 0;
int number_of_boss = 0; // 中ボス構造体の要素数(中ボス数)
char middle_boss[] = "";
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
		rom_flag = 1;
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

char* create_map(HWND hDlg, char* middle_str)
{
    int j = 0;
	int size = 0;  // 追加後の中ボスデータのサイズ
	long offset = 0;
	long insert_pos = 10;
	char size_middle[4];
	 
	// コンボボックスから文字列を取得
    offset = get_map_address_from_combo(hDlg);
    
    long new_size = map_size + strlen(middle_str);			 // map_sizeはascii時のサイズ
    char* buffer = ( char* )malloc(map_size+1);
	char* buffer2 = ( char* )malloc(map_size*2+1);		     // 16進数に変換するとasciiコードよりも２倍の文字数になる為 +1はnull文字
    char* new_buffer = (char* ) malloc(new_size + 1);
    char* new_buffer2 = (char* ) malloc(new_size*2 + 1);     // 16進数変換後
	
    fseek(fp,offset, SEEK_SET);								 // マップデータの位置までファイルポインタを移動
    fread(buffer, 1, map_size, fp);							 // map_sizeの大きさのマップデータをbufferへ読み込み
    insert_pos = seek_middle(buffer);						 // buffer(マップデータ)内の中ボスデータを追加するアドレスを調べる(16進数)
    middle_before = size_of_middle_boss(insert_pos,buffer);  // 中ボスデータを挿入する前の中ボスデータのサイズ(ascii)
    middle_bytes = strlen(middle_str);                       // 中ボスデータのサイズを取得
    //MessageBox(NULL,  middle_str, " middle_str", MB_OK);
    memcpy(new_buffer, buffer, insert_pos);					 // insert_pos(bufferの最初の位置から１６進数で何バイト中ボスを入れる位置まであるか）分だけbufferからnew_bufferへデータをコピー
	ascii_to_hex(new_buffer,new_buffer2,new_size);			 // new_bufferを16進数へ変換して new_buffer2へ入れる														 // insert_pos + 元の中ボスデータサイズの位置 へmiddle_boss(中ボスデータ)を、strlenの文字分だけコピー、追加
	memcpy(new_buffer2 + insert_pos  , middle_str, strlen(middle_str)); // 既存の中ボスデータを上書きしないように修正
    ascii_to_hex(buffer,buffer2, map_size );										// buffer (asciiのままのマップデータ)をbuffer2へasciiへ変換して入れる
						// insert_pos + 元の中ボスデータサイズの位置 + 中ボスデータの位置に、追加する前のマップデータ+insert_pos + middle_bytesからの残りのマップデータを入れる
	memcpy(new_buffer2 + insert_pos + strlen(middle_str), buffer2 + insert_pos +  middle_before*2 ,map_size*2 - insert_pos -  middle_before*2 );
	new_buffer2[insert_pos + middle_bytes + map_size*2 - insert_pos -  middle_before*2] = '\0';
	//MessageBox(NULL,  new_buffer2, " new_buffer2", MB_OK);
    free(buffer);
	free(buffer2);
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
	//MessageBox(NULL, index_out, "index_out", MB_OK);
	 itoa(offset,dummy_offset,16);
	 index_out[0] = dummy_offset[4];
	 index_out[1] = dummy_offset[5];
	 index_out[2] = dummy_offset[2];
	 index_out[3] = dummy_offset[3];
     index_out[5] = dummy_offset[1];
	hex_to_ascii(index_out,dummy_offset_out,8);
    dummy_offset[6] = '\0';
	//MessageBox(NULL, dummy_offset_out, "dummy_offset_out", MB_OK);
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

// 中ボスコンボボックスからリストボックスに追加
void add_to_list(HWND hDlg)
{
    char boss[32];
	int index = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
    
	if ( index == CB_ERR)
    {
         MessageBox(NULL, "中ボスを選択していません", "エラー", MB_ICONWARNING | MB_OK);
		 return;
	}

	SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETLBTEXT, index, (LPARAM)boss);
	SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0, (LPARAM)boss);
	number_of_boss++; // 中ボスのカウンタを1増やす
	return;
}

// 変更
void change_list(HWND hDlg)
{   
	char boss[32];
	int index = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
	int index_list = (int)SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETLBTEXT, index, (LPARAM)boss);
	SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_DELETESTRING, index_list , 0);
    SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_INSERTSTRING, index_list , (LPARAM)boss);
	return;
}

int insert_mboss(HWND hDlg)
{
     int offset = 0;
	 int dummy_flag = 0;
     char* write_address;
	 char addr[8];
	 int map_addr = 0;
	 int diff = 0; // 追加する前と後のバイト差
     
     diff = middle_bytes - middle_before;

     write_address = create_map(hDlg, middle_boss);  
	 map_addr = get_map_address_from_combo(hDlg);
    // マップデータの後に何バイトダミーデータが残っているか
    if(how_many_bytes_remain(hDlg) > diff)
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
	
    char* buffer = ( char* )malloc(map_size*2 + 1);
	hex_to_ascii(write_address,buffer,map_size+strlen(middle_boss) );
	buffer[strlen(write_address)*2 + 1] = '\0';
    fseek(fp,offset, SEEK_SET);                       // ダミーデータの位置まで移動（すでに入っている場合は動かさない）
    
    fwrite(buffer,map_size + middle_bytes/2 - middle_before ,1 ,fp);
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

	if(file_open_flag == 0) 
	{
          MessageBox(NULL, "ROMを開いてません", "情報", MB_ICONWARNING | MB_OK);
		  return 1;
	}

	fseek(fp,offset, SEEK_SET);
    while(i <  0xFFFF) // 0xFFFF回でループを抜ける
	{
		fread(byte,2,1,fp);
        byte[2] = '\0';
        ascii_to_hex(byte,buffer,2);
         fseek(fp,-1, SEEK_CUR);
        buffer[4] = '\0';
		//if(!strcmp(buffer,"EF3D")){ 
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
    offset =  get_map_address_from_combo(hDlg);
    
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

void strn_null(char* dest, const char* src, size_t num)
{
    strncpy(dest, src, num);
	dest[num] = '\0';
}

 // bufは中ボスデータの文字列
int read_middle(char* buf, DATA* data)
{
	int i = 0; int n = 0; 
	int k = 0; 

	char zero[10]; // 中ボスデータ判定用
	int num = 0; // 中ボスデータの個数

	// メタナイツ召喚などは除いておく...

	i += 4;  // 最初の4バイトはカット


	while (buf[i] != '\0')
	{
        strncpy(zero, buf + i, 10); 
        zero[10] = '\0';

		// 0が8個ある場合で判定
		if (!strcmp(zero, "0000000000") || !strcmp(zero, "0100000000"))
		{
           strn_null( data[n].zako, buf + i , 2);
		   if(atoi(data[n].zako) > 1)
		   {    
			   i += 2;
			   continue;  // 1か0以外の場合違うとみなす
		   }
           strn_null( data[n].defeat, buf + (i + 10), 2);
		   // ゲートスターの場合はスプライトに関するデータを省略
		   if (!strcmp(data[n].defeat, "01"))
		   {
			   // スプライトに関するデータは省略
               strn_null( data[n].defeat_x, buf + (i + 12), 4);
			   strn_null( data[n].defeat_y, buf + (i + 16), 4);
			   i += 20;
		   } // ワープスター
           else if (!strcmp(data[n].defeat, "02"))
		   {
               strn_null( data[n].sprite_action, buf + (i + 12), 2); // 撃破後のスプライトの行動パターン
               strn_null( data[n].defeat_x, buf + (i + 14), 4);
			   strn_null( data[n].defeat_y, buf + (i + 18), 4);
			   i += 22;
		   } // スプライト
		   else if (!strcmp(data[n].defeat, "03"))
		   {
               strn_null( data[n].sprite_action, buf + (i + 12), 2); // 撃破後のスプライトの行動パターン
			   strn_null( data[n].sprite, buf + (i + 14), 2);  // 撃破後のスプライト番号
               strn_null( data[n].defeat_x, buf + (i + 16), 4);
			   strn_null( data[n].defeat_y, buf + (i + 20), 4);
			   MessageBox(NULL,data[n].sprite , "data[n].sprite", MB_OK);
			   i += 24;
		   }
		   else if (!strcmp(data[n].defeat, "00"))
		   {
               i += 12;
		   }
		   else
		   {
               i += 2;
			   continue;
		   }
		   strn_null( data[n].number, buf + i, 2);    // 中ボスの数
           //MessageBox(NULL,data[n].number , "data[n].number", MB_OK);
		   int j = atoi(data[n].number);

		   while(1 <= j)
		   {
               strn_null( data[n].boss[k].action, buf + (i + 2), 2); // 中ボスの行動パターン
               strn_null( data[n].boss[k].sprite, buf + (i + 4), 2); // 中ボスのスプライト番号
		       strn_null( data[n].boss[k].boss_x, buf + (i + 6), 4); // 中ボスのx座標
		       strn_null( data[n].boss[k].boss_y, buf + (i + 10), 4);
			   i += 12;
               j--; k++;
		   }
		   n++;
		   k = 0;
		}
		i += 2;
	} 
    number_of_boss = n;
    return 0;
}

void add_boss_to_listbox(HWND hDlg)
{
	char* buffer = ( char* )malloc(map_size+1);
	
    if(load_map(hDlg))
	    return;
    int offset = get_map_address_from_combo(hDlg);
    fseek(fp,offset, SEEK_SET);	
	fread(buffer, 1, map_size, fp); // マップデータ読み込み
	int insert_pos = seek_middle(buffer);
    middle_bytes = size_of_middle_boss(insert_pos,buffer);
    char* buffer_boss = ( char* )malloc(middle_bytes+1);
	char* buffer_boss2 = ( char* )malloc(middle_bytes*2+1);  // 16進数に変換するとasciiコードよりも２倍の文字数になる為 +1はnull文字
	fseek(fp,offset + insert_pos / 2 , SEEK_SET);	
    fread(buffer_boss, 1, middle_bytes, fp); // 中ボスデータ読み込み
	ascii_to_hex(buffer_boss,buffer_boss2,middle_bytes);
	buffer_boss2[2*middle_bytes] = '\0';
	memset(data, 0, sizeof(data));
	read_middle(buffer_boss2, data);
	int n = sizeof(data) / sizeof(data[0]);
    listbox_number_of_boss(hDlg, data, n);
    
	return;
}

// リストボックスに中ボスを追加していく
void listbox_number_of_boss(HWND hDlg, DATA* data, int size)
{	
	SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_RESETCONTENT, 0, 0); // リストをクリア
    for ( int i = 0; i < size; i++)
	{
	    for ( int j = 0; j < 7; j++)
		{
            if (atoi(data[i].boss[0].sprite) == list[j].id)
		        SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_ADDSTRING, 0,  (LPARAM)list[j].name);
		}
    }
	return;
}

// 中ボスデータの文字列の長さを付加
void add_length(char *str)
{
	char num[5];
	char out[512] = "";
    int l = strlen(str);
	l = (l + 8) /2;
    int swap = ((l & 0xFF) << 8) | (( l >> 8) & 0xFF);
	sprintf(num, "%04X", swap);
    strcpy(out,num);
	strcat(out,str);
	strcat(out,"FFFF");

    if (sizeof(out) > 512)
	{
          MessageBox(NULL, "バッファオーバーフローが起こりました。", "エラー", MB_ICONWARNING | MB_OK);
		  exit(0);
	}
	strcpy(str, out);
    MessageBox(NULL, str, "str", MB_OK);
	return;
}

// 構造体から最終的に書き込む中ボスデータの文字列を作成
void create_middle (DATA* data, char *middle_str)
{
	int n = number_of_boss;
	char middle_info[32][128];
	char middle_boss[32][128];
	char out_info[32][512];
	char out_boss[256] = "";
    char _out[512] = "";

	for (int i = 0; i < n; i++)
	{
		 //MessageBox(NULL, data[i].defeat, "data[i].defeat", MB_OK);
		 int k = atoi(data[i].number);
		 if (!strcmp(data[i].defeat, "00"))
		     sprintf(middle_info[i],"%s00000000%s%s",data[i].zako, data[i].defeat, data[i].number);
		 else if (!strcmp(data[i].defeat, "01"))
		     sprintf(middle_info[i],"%s00000000%s%s%s%s",data[i].zako, data[i].defeat, data[i].defeat_x, data[i].defeat_y, data[i].number);
		 else if (!strcmp(data[i].defeat, "02"))
		     sprintf(middle_info[i],"%s00000000%s00%s%s%s",data[i].zako, data[i].defeat, data[i].defeat_x, data[i].defeat_y, data[i].number);
		 else if (!strcmp(data[i].defeat, "03"))
		     sprintf(middle_info[i],"%s00000000%s%s%s%s%s%s",data[i].zako, data[i].defeat, data[i].sprite_action ,data[i].sprite ,data[i].defeat_x, data[i].defeat_y, data[i].number);
         else
		 {
			 MessageBox(NULL, "中ボスデータ生成に失敗しました。", "エラー", MB_ICONWARNING | MB_OK);
			 return;
         }

		 for ( int j = 0; j < k; j++)
		 {
             sprintf(middle_boss[j],"%s%s%s%s",data[i].boss[j].action, data[i].boss[j].sprite, data[i].boss[j].boss_x, data[i].boss[j].boss_y);
			 strcat(out_boss, middle_boss[j]);
		 }
		 
		 strcpy(out_info[i], middle_info[i]);
		 strcat(out_info[i], out_boss);
		 strcpy(out_boss, ""); // out_bossを空にする
	}

	for ( i = 0; i < n; i++)
	{
        strcat(_out, out_info[i]);
	}
    add_length(_out);
	strcpy(middle_str, _out);
}
// sort and convert hex to decimal
void convert_and_set(char *str, HWND hDlg, int IDC)
{
    char temp[5];
	char out[6];
	temp[0] = str[2];
	temp[1] = str[3];
	temp[2] = str[0];
	temp[3] = str[1];
	temp[4] = '\0';
    sprintf(out, "%ld", strtol(temp, NULL, 16));
	SetDlgItemText(hDlg, IDC, TEXT(out));
}

// sort and convert decimal to hex
void convert_decimal_to_hex(char *str, char *out)
{
    int value = 0;
	int swapped = 0;
    value = strtoul(str, NULL, 10);  // 基数を10進数としてint型へ変換
    swapped = ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
    sprintf(out, "%04X", swapped);
}

// 構造体から選択中の中ボスを削除、して削除して空いた構造体配列を詰める
void delete_struct(DATA* data, int number, HWND hDlg)
{
    memset(&data[number], 0, sizeof(data[0]));

	for ( int i = number; i < number_of_boss; i++)
	{
        data[i] = data[i + 1];
	}
	
}

void change_struct(DATA* data, int number, HWND hDlg)
{
	int i = 0;
    char boss[32], defeat_number[32];
	char x_axis[5];
	char y_axis[5];
	char x_axis2[5];
	char y_axis2[5];
    char action_defeat[5];
	char sprite_defeat[5];

	int index = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETCURSEL, 0, 0);
	int index2 = (int)SendMessage(GetDlgItem(hDlg, IDC_COMBO6), CB_GETCURSEL, 0, 0);
	SendMessage(GetDlgItem(hDlg, IDC_COMBO2), CB_GETLBTEXT, index, (LPARAM)boss); // boss配列に選択した中ボス文字列を格納
	SendMessage(GetDlgItem(hDlg, IDC_COMBO6), CB_GETLBTEXT, index2, (LPARAM)defeat_number);
    GetDlgItemTextA(hDlg,IDC_EDIT1,x_axis,4); // x_axis配列へIDC_EDIT1の文字列を格納
    GetDlgItemTextA(hDlg,IDC_EDIT2,y_axis,4); // 
	GetDlgItemTextA(hDlg,IDC_EDIT5,sprite_defeat,2);
    GetDlgItemTextA(hDlg,IDC_EDIT6,action_defeat,2);
    
    for ( i = 0; i < sizeof(list) / sizeof(list[0]); i++)
	{
        if(!strcmp(boss,list[i].name))
		{
		    char id[4];
		    itoa(list[i].id, id, 10);
            strcpy(data[number].boss[0].sprite,id);
		}
	}

	convert_decimal_to_hex(x_axis, data[number].boss[0].boss_x);
	convert_decimal_to_hex(y_axis, data[number].boss[0].boss_y);
	strcpy(data[number].boss[0].action, "00");

	// 雑魚キャラを残すにチェックが入っている場合
    if(BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK2) , BM_GETCHECK , 0 , 0))
	{
         strcpy(data[number].zako, "01");
	}
	else
	{
         strcpy(data[number].zako, "00");
	}

	// 撃破後の処理にチェックが入っている場合
	if(BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK , 0 , 0))
	{
         convert_decimal_to_hex(x_axis2, data[number].defeat_x);   // 撃破後の処理のx座標, ｙ座標
		 convert_decimal_to_hex(y_axis2, data[number].defeat_y);
		 
		 for ( i = 0; i < sizeof(defeat) / sizeof(defeat[0]); i++)
		 {
             if(!strcmp(defeat_number,defeat[i].name))
			 {
                  char id_defeat[4];
		          itoa(defeat[i].defeat_id, id_defeat, 10);
                  strcpy(data[number].defeat,id_defeat); 
				  data[number].defeat[1] = data[number].defeat[0];
                  data[number].defeat[0] = '0';
			 }
		 }
         MessageBox(NULL, data[number].defeat, "data[number].defeat", MB_OK);
		 if (!strcmp(data[number].defeat,"01"))
		 {
             // ゲートスターの場合はスプライトのアクションは省略
		 }
		 else if (!strcmp(data[number].defeat,"02"))
		 {
              strcpy(data[number].sprite_action, "00"); //ワープスターの場合はスプライトのアクションは00
			  // スプライトは省略
		 } // 特定のスプライト
		 else if (!strcmp(data[number].defeat,"03"))
		 {
              strcpy(data[number].sprite_action, action_defeat);
			  strcpy(data[number].sprite, sprite_defeat);
			  MessageBox(NULL, data[number].sprite, "data[number].sprite", MB_OK);
		 }
         
	} // 撃破後の処理なし
	else
	{
         strcpy(data[number].defeat, "00");
	}
	strcpy(data[number].number,"01");  // 出現数

	//number++;
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
	char id[4];
	char defeat_id[2];
	int size_x, size_y;
	int i = 0;
	int l = 0;
	int index = 0;
	int zako = 0;
	char sprite_action[2], sprite[2];

	LPCSTR number[3] = {"1","2","3"};
	
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
		if( LOWORD(wParam) == IDC_BUTTON2 )
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
			// 撃破後の処理のチェックが入っているなら 
            if (SendMessage(checkbox, BM_GETCHECK, 0, 0) == BST_CHECKED)
			{
                //EnableWindow(GetDlgItem(hDlg, IDC_EDIT3), TRUE);
			}
		}
        if( LOWORD(wParam) == IDC_BUTTON3)
		{
          
		}
        if (LOWORD(wParam) == IDC_BUTTON9)
		{
			if (SendMessage( GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0) == -1)
			{
                MessageBox(NULL, "マップアドレスを追加してください。", "エラー", MB_ICONWARNING | MB_OK);
				break;
			}

             add_boss_to_listbox(hDlg);
		}
		if (LOWORD(wParam) == IDC_BUTTON1 || LOWORD(wParam) == 40002) 
		{
		   if( file_open())
		   {
               SetDlgItemTextA(hDlg, IDC_STATIC2, rom_name);
		       map_address(hDlg);
		   }
		} 
		if (LOWORD(wParam) == 40004)
		{
            if (!rom_flag)
			{
                 MessageBox(NULL, "ROMを開いてません", "エラー", MB_ICONWARNING | MB_OK);
			}

			if (SendMessage( GetDlgItem(hDlg, IDC_COMBO1), LB_GETCURSEL, 0, 0) == LB_ERR)
			{
                MessageBox(NULL, "中ボスデータを読み込んでください。", "エラー", MB_ICONWARNING | MB_OK);
				break;
			}
            create_middle(data, middle_boss);
			break;
		}
		//追加ボタン
		if (LOWORD(wParam) == IDC_BUTTON5)
		{
            int n = sizeof(data) / sizeof(data[0]);

            if (SendMessage( GetDlgItem(hDlg, IDC_COMBO1), CB_GETCURSEL, 0, 0) == -1)
			{
                MessageBox(NULL, "マップアドレスを追加してください。", "エラー", MB_ICONWARNING | MB_OK);
				break;
			}

			if (SendMessage( GetDlgItem(hDlg, IDC_COMBO1), LB_GETCURSEL, 0, 0) == LB_ERR)
			{
                MessageBox(NULL, "中ボスデータを読み込んでください。", "エラー", MB_ICONWARNING | MB_OK);
				break;
			}
            
			/* 
			   構造体に新たにユーザから設定されたデータに基づき中ボスデータを追加...
			*/
			change_struct(data, number_of_boss, hDlg);
			// リストボックスの最後に設定した中ボスを追加
            add_to_list(hDlg);
			break;
		} // 変更ボタン
        if (LOWORD(wParam) == IDC_BUTTON6)
		{
			/* 
			   構造体に新たにユーザから設定されたデータに基づき中ボスデータを変更...
			*/
            index = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
            change_struct(data, index, hDlg);
			//  リストボックスで選択された中ボスを変更
			change_list(hDlg);
            break;
		} // 削除ボタン
		if (LOWORD(wParam) == IDC_BUTTON7)
		{
			/*
               構造体から中ボスデータを削除
            */
			index = SendMessage(GetDlgItem(hDlg, IDC_LIST1), LB_GETCURSEL, 0, 0);
			delete_struct(data, index, hDlg);                // リストボックスで選択された中ボスを削除
			int n = sizeof(data) / sizeof(data[0]);
            listbox_number_of_boss(hDlg, data, n);
            number_of_boss--;
            break;
		} // リストボックスの要素を選択したときにコンボボックスとエディタボックスの内容を読み込んだ中ボスデータに変更
		if (LOWORD(wParam) == IDC_LIST1)
		{
             if (HIWORD(wParam) == LBN_SELCHANGE)
			 {
                int index = (int)SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCURSEL, 0, 0); // リストボックス内の中ボスのリストのうち

                // エディットボックスへ設定
				convert_and_set(data[index].boss[0].boss_x,hDlg, IDC_EDIT1);
                convert_and_set(data[index].boss[0].boss_y,hDlg, IDC_EDIT2);
                // コンボボックスへ設定
                int sprite = atoi(data[index].boss[0].sprite);
				int _defeat = atoi(data[index].defeat);

				for ( int i = 0; i < sizeof(data) / sizeof(data[0]); i++)
				{
                     if ( sprite == list[i].id)
						 break;
				}

				SendDlgItemMessage(hDlg, IDC_COMBO2, CB_SETCURSEL, i, 0);

                for ( i = 0; i < sizeof(data) / sizeof(data[0]); i++)
				{
                     if ( _defeat == defeat[i].defeat_id)
						 break;
				}
                SendDlgItemMessage(hDlg, IDC_COMBO6, CB_SETCURSEL, i, 0);

				convert_and_set(data[index].defeat_x,hDlg, IDC_EDIT3);
                convert_and_set(data[index].defeat_y,hDlg, IDC_EDIT4);
			 }
		}
		if (LOWORD(wParam) == IDC_COMBO6)
		{
            if (HIWORD(wParam) == CBN_SELCHANGE)
			{
                 
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