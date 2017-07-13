#include <stdio.h>
#include <math.h>

#define user_number 1000       //ユーザーの数
#define movie_number 1000      //映画の数
#define k 5                    //近傍k

int func(FILE *fp_r,FILE *fp_w){
    
    double dataset[user_number][movie_number];
    float sim[user_number][2];                    //sim[][0]はuserid,sim[][1]は類似度を入れる
    float Comparison_ave[user_number];            //各ユーザーの平均値
    int userid,movieid,rec_user,ret,i,j,a,best_movie = 0;
    double ratings,tmp;
    float timestamp,element1,element2,element3,rec_sum,rec_ave;
    float Comparison_sum,sim_sum_molecule,sim_sum_denominator,max;
    
    /*データセット配列の初期化*/
    for(i = 0; i < user_number; i++){
        for(j = 0; j < user_number; j++){
            dataset[i][j] = 0.00000;
        }
    }
    
    /*ratings.csvからデータを読み取る*/
    while ((ret = fscanf(fp_r, "%d,%d,%lf,%f,", &userid, &movieid, &ratings, &timestamp)) != EOF ){
        //printf("%d,%d,%f\n",userid,movieid,ratings);
        if(userid <= user_number && movieid <= movie_number){              //設定した分のデータのみをデータセット配列に入れる
            dataset[userid-1][movieid-1] = ratings;
        }
    }
    
    /*読み込んだ結果の出力（いらない）*/
    fprintf(fp_w,"user_id,movie,\n");
    for(i = 0; i < user_number; i++){
        fprintf(fp_w,"%d,",i+1);
        for(j = 0; j < user_number; j++){
            //printf("user %d/movie %d/ ratings /%lf\n",i+1,j+1,dataset[i][j]);
            fprintf(fp_w,"%lf,",dataset[i][j]);
            if(j == user_number - 1) fprintf(fp_w,"\n");
        }
    }
    
    /*おすすめを受けるユーザーの入力*/
    printf("user id?\n");
    scanf("%d",&rec_user);
    
    
    rec_sum = 0.0;
    for(j = 0; j < user_number; j++){
        rec_sum += dataset[rec_user-1][j];
        //printf("%f\n",rec_sum);
    }
    rec_ave = rec_sum / user_number;            //おすすめするユーザーの映画の評価平均値
    
    /*ピアソン類似度の計算*/
    for(i = 0; i < user_number; i++){
        element1 = element2 = element3 = 0.0;
        Comparison_sum = 0.0;
        for(j = 0; j < movie_number; j++)    Comparison_sum += dataset[i][j];
        Comparison_ave[i] = Comparison_sum / movie_number;
        
        for(j = 0; j < movie_number; j++){
            element1 += (dataset[rec_user-1][j]-rec_ave) * (dataset[i][j]-Comparison_ave[i]);
            element2 += pow(dataset[rec_user-1][j],2);
            element3 += pow(dataset[i][j],2);
            //printf("%f,%f,%f\n",element1,element2,element3);
        }
        
        if(element3 == 0)   element3 = 0.00001;                            //nan回避
        //printf("%f,%f,%f\n",element1,sqrt(element2),sqrt(element3));
        sim[i][0] = i+1;
        sim[i][1] = element1 /(sqrt(element2)*sqrt(element3));                 //ピアソン類似度の計算
        //printf("user %1.0f is similarity = %f\n",sim[i][0],sim[i][1]);
    }
    
    /*類似度の高い上位kユーザを抽出*/
    
    /*STEP1:類似度が高い順に並べる*/
    for(i = 0; i < user_number; i++){
        for(j = i+1; j < user_number; j++){
            if (sim[i][1] < sim[j][1]){
                tmp = sim[i][1];
                sim[i][1] = sim[j][1];
                sim[j][1] = tmp;
                tmp = sim[i][0];
                sim[i][0] = sim[j][0];
                sim[j][0] = tmp;
            }
        }
    }
    for(i = 1; i <= k; i++){
        printf("user %1.0f's similarity = %f\n",sim[i][0],sim[i][1]);
    }
    
    /*STEP2:加重平均を用いて、まだ評価していない映画の評価値を予測する*/
    //printf("%f\n",rec_ave);
    max = 0.0;
    
    for(j = 0; j < user_number; j++){
        if(dataset[rec_user-1][j] == 0){       //おすすめを受けるユーザーがまだ評価していない映画の探索
            sim_sum_molecule = sim_sum_denominator = 0.0;
            for(i = 1; i <= k; i++){
                a = sim[i][0];
                //printf("%d\n",a);
                //printf("ユーザー:%d,このムービーの評価値:%2.1f,このユーザーの評価の平均値%f\n",a,dataset[a-1][j],Comparison_ave[a-1]);
                sim_sum_molecule += sim[i][1] * (dataset[a-1][j] - Comparison_ave[a-1]);
                sim_sum_denominator += sim[i][1];
            }
            //printf("%f,%f,%f\n",rec_ave,sim_sum_molecule,sim_sum_denominator);
            dataset[rec_user-1][j] = rec_ave + sim_sum_molecule / sim_sum_denominator;          //加重平均
            
            /*STEP3:一番評価値が高かったものをおすすめする*/
            if(dataset[rec_user-1][j] > max){
                max = dataset[rec_user-1][j];
                best_movie = j+1;
                //printf("%f,%d\n",max,best_movie);
            }
        }
        printf("movie:%d    %f\n",j+1,dataset[rec_user-1][j]);
    }
    
    printf("%d movies are recommended for %d user\n",best_movie,rec_user);
    return 0;
}

int main(){
    
    FILE *fp_r,*fp_w;
    
    fp_r = fopen("ratings.csv","r");
    fp_w = fopen("print.csv","w");
    
    if(fp_r == NULL ){
        printf( "ファイルが開けません\n");
    }
    func(fp_r,fp_w);
    
    fclose(fp_r);
    fclose(fp_w);
    
    return 0;
    
}
