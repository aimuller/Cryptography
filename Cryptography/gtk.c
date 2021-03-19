#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<gtk/gtk.h>
#include <time.h>
#include <glib.h>
long sum,now,num;
void proc1(int argc,char *argv[]);
void proc2(int argc,char *argv[]);
void proc3(int argc,char *argv[]);
gboolean update_time(gpointer data){
    time_t times;
    struct tm *p_time;
    //time()函数获得计算机系统当前的日历时间
    time(&times);
    //将日历时间数值变换成本地时间
    p_time = localtime(&times);
    //拼接日期和时间字符串
    gchar *text_data = g_strdup_printf("日期：%04d-%02d-%02d",(1900+p_time->tm_year),(1+p_time->tm_mon),(p_time->tm_mday));
    gchar *text_time = g_strdup_printf("时间：%02d:%02d:%02d",(p_time->tm_hour), (p_time->tm_min), (p_time->tm_sec));
    gchar *text_last = g_strdup_printf("<span size='x-large'>\n%s\n\n%s\n</span>", text_time, text_data);
    gtk_label_set_markup(GTK_LABEL(data), text_last);
    return TRUE;
}
gboolean update_num(gpointer data){
    gchar *text = g_strdup_printf("<span size='x-large'>显示数字：%ld\n</span>",num);
    gtk_label_set_markup(GTK_LABEL(data), text);
    num=(num+1)%10;
    return TRUE;
}
gboolean update_add(gpointer data){
    gchar *text = g_strdup_printf("<span size='x-large'>当前数字：%ld\n上次累积和：%ld\n本次累积和：%ld+%ld=%ld\n</span>",now,sum,now,sum,sum+now);
    gtk_label_set_markup(GTK_LABEL(data), text);
    sum+=now;
    now++;
    return TRUE;
}
int main(int argc,char *argv[]){
    pid_t pid1,pid2,pid3;
    sum=0;now=1;num=0;
    if(pid1=fork()==0)
        proc1(argc,argv);
    else{
        if( pid2=fork()==0)
            proc2(argc,argv);
        else{
            proc3(argc,argv);
        } 
    }
    return 0;
}

void proc1(int argc,char *argv[]){
    GtkWidget *window;
    GtkWidget *label;

    gtk_init(&argc,&argv);
    
    //创建窗口和显示标签
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    label = gtk_label_new(NULL);

    //设置window标题,大小和位置
    gtk_window_set_title(GTK_WINDOW(window),"显示系统时间");
    gtk_widget_set_size_request(window, 400,400);
    gtk_window_move(GTK_WINDOW(window), 200,200);

    //收到delete信号，gtk库自带的回调函数gtk_main_quit关闭窗口
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(gtk_main_quit),NULL);

    //将label包含在window中
    gtk_container_add(GTK_CONTAINER(window),label);
    
    //每隔1s刷新系统时间
    gint s = g_timeout_add (1000, update_time, (void *)label);

    //显示所有控件
    gtk_widget_show_all(window);
    gtk_main();

}
void proc2(int argc,char *argv[]){
    GtkWidget *window;
    GtkWidget *label;

    gtk_init(&argc,&argv);
    
    //创建窗口和显示标签
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    label = gtk_label_new(NULL);

    //设置window标题,大小和位置
    gtk_window_set_title(GTK_WINDOW(window),"0~9循环显示");
    gtk_widget_set_size_request(window, 400,400);
    gtk_window_move(GTK_WINDOW(window), 700,200);

    //收到delete信号，gtk库自带的回调函数gtk_main_quit关闭窗口
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(gtk_main_quit),NULL);

    //将label包含在window中
    gtk_container_add(GTK_CONTAINER(window),label);
    
    //每隔1s刷新系统时间
    gint s = g_timeout_add (1000, update_num, (void *)label);

    //显示所有控件
    gtk_widget_show_all(window);
    gtk_main();

}
void proc3(int argc,char *argv[]){
    GtkWidget *window;
    GtkWidget *label;

    gtk_init(&argc,&argv);
    
    //创建窗口和显示标签
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    label = gtk_label_new(NULL);

    //设置window标题,大小和位置
    gtk_window_set_title(GTK_WINDOW(window),"0～1000累加");
    gtk_widget_set_size_request(window, 400,400);
    gtk_window_move(GTK_WINDOW(window), 1200,200);

    gtk_widget_set_size_request(label, 300, 200);

    //收到delete信号，gtk库自带的回调函数gtk_main_quit关闭窗口
    g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(gtk_main_quit),NULL);

    //将label包含在window中
    gtk_container_add(GTK_CONTAINER(window),label);
    
    //每隔1s刷新系统时间
    gint s = g_timeout_add (1000, update_add, (void *)label);

    //显示所有控件
    gtk_widget_show_all(window);
    gtk_main();

}
