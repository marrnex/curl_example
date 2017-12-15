/*
ポケモンの種族値をcsvに書き込むプログラム.
以下のファイルが生成される
	status.csv:ポケモンの種族値が書き込まれている
	zukan.html:参照したサイトのhtmlコード
	utf8.txt:zukan.htmlをutf-8にエンコードしたファイル

実行方法:
	$ gcc urltest.c -l curl -liconv
	でa.outの実行ファイルが生成されるので
	./a,out
	で実行
*/


/* -*- coding: utf-8 -*- */
#include <curl/curl.h>
#include <stdio.h>
#include <iconv.h>
#include <string.h>
#define S_SIZE (1028)
#define POKENUM 806 //現在のポケモンの最終番号

int main(int argc, char *argv[])
{
	CURL *curl;//curlやるときに使う変数(よくわかってない)
	CURLcode res;//同上
	char std_add[256]="https://yakkun.com/sm/zukan/n",poke_add[256],pokenum[5];//std_add:基本となるurlを格納してある。poke_add:std_addに図鑑番号を付属するための変数,pokenum:図鑑番号を文字列にした結果を格納する
	char c[256];//ファイルから読み込んだデータを格納する変数
	int i=0;

	iconv_t icd;//エンコードライブラリiconvで使う変数。
  	FILE *fp_src, *fp_dst;//エンコード前ファイルを読み込み、エンコード後ファイルを書き込みするためのファイルポインタ
  	char s_src[S_SIZE], s_dst[S_SIZE];
  	char *p_src, *p_dst;
  	size_t n_src, n_dst;

  	int pokecou;//現在アクセスする図鑑番号

  	FILE *fp2;//最終的に作るcsvファイルのポインタ
	if((fp2=fopen("status.csv","w"))==NULL){//開けるか確認(無かったら勝手に作るしほとんどいらない)
		printf("connot open!");
		return -1;
	}
	fprintf(fp2, "num,name,HP,A,B,C,D,S\n");//表の各要素のタイトル

  	for(pokecou=1;pokecou<=100;pokecou++){//No.1からNo.100のポケモンまで繰り返す

	    //////////////////////////////////htmlファイル作成部////////////////////////////////////////////////////

		curl = curl_easy_init(); //ハンドラの初期化

		sprintf(pokenum,"%d",pokecou);//現在のループ回数(pokecou)を文字列型(pokenum)に変換
		strcpy(poke_add,std_add);//poke_addに番号前アドレス("https://yakkun.com/sm/zukan/n")をいれる
		strcat(poke_add,pokenum);//上の文字列に文字列化した図鑑番号を追加してpoke_addにいれる
		printf("%s",poke_add);//作ったアドレスの確認

		curl_easy_setopt(curl, CURLOPT_URL, poke_add); //URLの登録

		FILE *fp = fopen("zukan.html", "w"); //取得したデータを書き込むファイル、"f_name"はファイル名
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite); //BODYを書き込む関数ポインタを登録
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //↑で登録した関数が読み取るファイルポインタ
		res = curl_easy_perform(curl); //いままで登録した情報で色々実行

		long http_code = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code); //ステータスコードの取得
		curl_easy_cleanup(curl); //ハンドラのクリーンアップ
		fclose(fp);
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////エンコード部/////////////////////////////////////////////////////////////
		icd = iconv_open("UTF-8","EUC-JP");//EUC-JPをUTF-8にするっていう宣言(さっき作ったhtmlがEUC-JPであるため)
		if((fp_src = fopen("zukan.html", "r"))==NULL){//さっき作ったhtmlファイルを開く
		  puts("cannot open");
		  return -1;
		}
		fp_dst = fopen("utf8.txt", "w");//エンコード後の結果を書き込むファイルを開く

		while(1){
			fgets(s_src, S_SIZE, fp_src);//fp_srcからデータをとってくる

		  	if (feof(fp_src))//取れてなかったらおわり
		    	break;
		  	p_src = s_src;
		  	p_dst = s_dst;
		  	n_src = strlen(s_src);
		  	n_dst = S_SIZE-1;

		  	iconv(icd, &p_src, &n_src, &p_dst, &n_dst);//エンコードする

		    *p_dst = '\0';
		    fputs(s_dst, fp_dst);
		  }

		  fclose(fp_dst);
		  fclose(fp_src);
		  iconv_close(icd);


		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////htmlファイル解析部////////////////////////////////////////////////////

		fprintf(fp2,"%d,",pokecou);//図鑑ナンバーをcsvに書き込む
		if((fp=fopen("utf8.txt","r"))!=NULL){//エンコードしたファイルを開く
			while(fgets(c,256,fp)!=NULL){//データをとってくる
				i=0;//文字比較で用いる変数初期化
				if(strstr(c,"<title>")!=NULL){//とってきたもじれつに<titele>の文が存在したら

					//////////////////////ココらへん名前引き出すための儀式みたいな感じなので分からなかったらhtmlソース見て下さい(もっといいやり方あると思う)///////////////
					while(c[i]!='>')//'>'が出てくるまで進む
						i++;
						i++;//さらに1文字進む
					while(c[i]!='&'){//&が出るまでポケモンの名前なのでそこまで書き込む
						printf("%c",c[i]);//コレ日本語の場合2つ分で1文字になるので注意(他は1つで1文字分)
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					fprintf(fp2,",");//区切る
				}
				if(strstr(c,"HP")!=NULL){//取ってきた文字列に"HP"の文字が存在したら(HPの種族値を抜き出すため)
					///////////種族値手前まで移動する///////////
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					/////////////////////////////////////////
					while(c[i]>=48&&c[i]<=57){//数値が出てる間は種族値なので書き込む(48はchar型でいう0,57はchar型でいう9に該当)
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					printf(",");
					fprintf(fp2,",");//区切る
				}
				if(strstr(c,"こうげき")!=NULL){//取ってきた文字列に"こうげき"の文字が存在したら(こうげきの種族値を抜き出すため)
					///////////種族値手前まで移動する///////////
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					/////////////////////////////////////////
					while(c[i]>=48&&c[i]<=57){//数値が出てる間は種族値なので書き込む(48はchar型でいう0,57はchar型でいう9に該当)
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					printf(",");
					fprintf(fp2,",");//区切る
				}
				if(strstr(c,"ぼうぎょ")!=NULL){//取ってきた文字列に"ぼうぎょ"の文字が存在したら
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]>=48&&c[i]<=57){
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					printf(",");
					fprintf(fp2,",");
				}
				if(strstr(c,"とくこう")!=NULL){
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]>=48&&c[i]<=57){
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					printf(",");
					fprintf(fp2,",");
				}
				if(strstr(c,"とくぼう")!=NULL){
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]>=48&&c[i]<=57){
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					printf(",");
					fprintf(fp2,",");
				}
				if(strstr(c,"すばやさ")!=NULL){
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]!=';')
						i++;
					i++;
					while(c[i]>=48&&c[i]<=57){
						printf("%c",c[i]);
						fprintf(fp2,"%c",c[i]);
						i++;
					}
					fprintf(fp2,"\n");
					break;//全部種族値取れたのでループからぬける
				}
			}
		puts("");

		fclose(fp);
		}else{
			puts("cannot open!");
		}
	}
	fclose(fp2);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	return 0;
}
