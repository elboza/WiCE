#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>
#include<gtk/gtk.h>
#include"main.h"
#include"list_utils.h"
//#include"parse2.h"
#include"pack.h"
//#include"init_game.h"
#include"scheduler.h"
#include"execute.h"
#include"debug_output.h"
#include"txt_output.h"
#include"x11_output.h"

GtkWidget *window1,*window2,*window3;
GtkWidget *vbox;
GtkWidget *drawing_area;
GdkDrawable *main_map;
GtkWidget *hbox;
GtkWidget *liststore_instr;
GtkWidget *liststore_warrior;
GtkWidget *scrolledwindow1,*scrolledwindow2,*scrolledwindow3;
GtkWidget *statusbar;
GtkWidget *warriorview,*instrview;
GtkTreeIter iter;
gint context_id,timeout_tag,gtkhistory,gtkwarrior;
GtkListStore *model,*model2;              /* l'oggetto model   */
  
  GtkWidget *view,*view2;                         /* -\                  */ 
  GtkCellRenderer *renderer;                          /* --> l'oggetto view  */
  GtkTreeSelection *selection,*selection2;      /* -/                  */
struct process_task *gtktask;
static GdkPixmap *pixmap = NULL;
GdkGC* execute_gc;
GdkColor black;
GdkColormap *execute_colormap;	
GdkPixbuf *pixbuf;
GdkPixmap *bullet;

struct process_task* get_cell_owner(int addr)
{
	struct unpacked_op_mem mem;
	struct process_task *mtask;
	unpack(addr,&mem);
	if(mem.processID==0) return NULL;
	mtask=primo_task;
	do{
		if(mem.processID==mtask->ID) return mtask;
		mtask=mtask->next;
	}while(mtask);
	//mtask=first_killed_task;
	//do{
	//	if(mem.processID==mtask->ID) return mtask;
	//	mtask=mtask->next;
	//}while(mtask);
	return NULL;
}
static void color_icon (GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha)
{
  int width, height, rowstride, n_channels,i,j;
  guchar *pixels, *p;

  n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  g_assert (gdk_pixbuf_get_colorspace (pixbuf) == GDK_COLORSPACE_RGB);
  g_assert (gdk_pixbuf_get_bits_per_sample (pixbuf) == 8);
  g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
  g_assert (n_channels == 4);

  width = gdk_pixbuf_get_width (pixbuf);
  height = gdk_pixbuf_get_height (pixbuf);

  g_assert (x >= 0 && x < width);
  g_assert (y >= 0 && y < height);

  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  pixels = gdk_pixbuf_get_pixels (pixbuf);

  for(i=0;i<10;i++)
  for(j=0;j<10;j++)
  {
  p = pixels + j * rowstride + i * n_channels;
  p[0] = red;
  p[1] = green;
  p[2] = blue;
  p[3] = alpha;
  }
  
  p = pixels + y * rowstride + x * n_channels;
  p[0] = red;
  p[1] = green;
  p[2] = blue;
  p[3] = alpha;
  gdk_pixbuf_fill(pixbuf,(guint32)p);
}
static guint32 get_RGBA(guchar red, guchar green, guchar blue)
{
	guchar *p,*pixels;
	int x=0,y=0,n_channels,rowstride;
	n_channels = gdk_pixbuf_get_n_channels (pixbuf);
	rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  	pixels = gdk_pixbuf_get_pixels (pixbuf);
  	p = pixels + y * rowstride + x * n_channels;
  	p[0] = red;
  	p[1] = green;
  	p[2] = blue;
  	p[3] = 1;
	return *p;
}
static void on_destroy (GtkWidget * widget, gpointer data)
{
    gtk_main_quit ();
    sprintf(out_str,"User Termination.\n");
    fputs(out_str,fpout);
    exit(1);
}
static void on_history_destroy (GtkWidget * widget, gpointer data)
{
    gtkhistory=0;
}
static void on_warrior_destroy (GtkWidget * widget, gpointer data)
{
    gtkwarrior=0;
}
void gtk_display_curr_instr(char *ss)
{
	if(gtkhistory)
	{
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  		gtk_list_store_set(GTK_LIST_STORE(model), &iter,0, ss,-1);
  		//and scoll down !!!!
  		//gtk_signal_emit_by_name(GTK_OBJECT(scrolledwindow2),"scroll_event",NULL);
  	}
}
gint gtk_statistic(gpointer data)
{
	
	return TRUE;
}
void gtk_update_warrior(struct process_task *ptask)
{
	int x,n;
	char s1[MAXSTR];
	n=ptask->ID - 1;
	x=gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model2),&iter,NULL,n);
	if(x)
	{
		//gtk_list_store_set(GTK_LIST_STORE(model2), &iter,0, pixbuf,1,"ciao",2,out_str,-1);
		//gtk_list_store_set(GTK_LIST_STORE(model2), &iter,0, NULL,1,"pippo",2,"asas",-1);
		pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,10,10);
		sprintf(out_str,"#%d (%c) color(%d,%d,%d)",ptask->ID,ptask->out_symbol,ptask->m_color.red,ptask->m_color.green,ptask->m_color.blue);
		bullet = gdk_pixmap_new (drawing_area->window, 10,10, -1);
		gdk_gc_set_foreground (execute_gc, &(ptask->m_color));
		gdk_draw_rectangle (bullet, execute_gc, TRUE, 0, 0,10,10);
		gdk_pixbuf_get_from_drawable(pixbuf,bullet,NULL,0,0,0,0,10,10);
		if(ptask->n_threads)
		{
			sprintf(s1,"%d",ptask->n_threads);
		}
		else
		{
			sprintf(s1,"Dead!");
		}
		gtk_list_store_set(GTK_LIST_STORE(model2), &iter,0, pixbuf,1,s1,2,out_str,-1);
		g_object_unref(bullet);
		g_object_unref(pixbuf);
	}
}
static gint main_expose (GtkWidget *widget, GdkEventExpose *event)
{
  int x1,y1,x2,y2;
  int i;
	struct process_task *mtask;
  gdk_gc_set_foreground (execute_gc, &black);
  gdk_draw_rectangle (widget->window, widget->style->black_gc, TRUE, event->area.x, event->area.y,event->area.width,event->area.height);
	for(i=0;i<size_arena;i++)
	{
		mtask=get_cell_owner(i);
		if(mtask)x11_cell_refresh(i,mtask->cur_thread);
		else x11_cell_refresh(i,NULL);
	}
	//gdk_draw_rectangle(widget->window,execute_gc,TRUE,10,10,5,5);
  return TRUE; /* Why do we need this??? */
}
static gint main_configure (GtkWidget *widget, GdkEventConfigure *event)
{
  main_map = widget->window;
  execute_gc = gdk_gc_new (drawing_area->window);
  execute_colormap = gdk_window_get_colormap (drawing_area->window);
  black.red = 15000;
  black.green = 15000;
  black.blue = 15000;
  gdk_color_alloc (execute_colormap, &black);      
  gdk_gc_set_background (execute_gc, &black);

  return FALSE;
}
void x11_cell_refresh(int addr,struct process_thread *pt)
{
	int x,y;
	x=(addr%gtk_sc_x)*6;
	y=(int)(((double)addr/(double)gtk_sc_x))*6;
	if(pt==NULL)
	{
		gdk_gc_set_foreground(execute_gc,&black);
	}
	else
	{
		gdk_gc_set_foreground(execute_gc,&(pt->ptask->m_color));
	}
	gdk_draw_rectangle(main_map,execute_gc,TRUE,x,y,5,5);
}
gint gtk_execute(gpointer data)
{
	struct process_task *task_to_kill;
	int alive;
	static int actual_CPU=0;
	if(actual_CPU++>=CPU_cicle)
	{
		if(gtkhistory)
		{
			//gtk_display_draw
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			sprintf(out_str,"This is a draw !!!");
  			gtk_list_store_set(GTK_LIST_STORE(model), &iter,0, out_str,-1);
  		}
		context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar),"my_statusbar");
		gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, "(Not Running), this is a draw!");
		return FALSE;
	}
	if(primo_task==ultimo_task)
	{
		if(gtkhistory)
		{
			//gtk_display_winner
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			sprintf(out_str," ...and the winner is #%d (%c) after %d CPU cicles",primo_task->ID,primo_task->out_symbol,actual_CPU);
  			gtk_list_store_set(GTK_LIST_STORE(model), &iter,0, out_str,-1);
  		}
		context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar),"my_statusbar");
		sprintf(out_str,"(Not Running), ...and the winner is #%d (%c) after %d CPU cicles",primo_task->ID,primo_task->out_symbol,actual_CPU);
		gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id,out_str);
		return FALSE;
	}
	alive=step(gtktask->cur_thread);
	if(alive)
	{
		gtktask->cur_thread=gtktask->cur_thread->next;
		if(gtktask->cur_thread==NULL) gtktask->cur_thread=gtktask->primo_thread;
	}
	else
	{
		del_thread(gtktask->cur_thread);
		if(gtkhistory)
		{
			//gtk_display thread dead
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			sprintf(out_str,"a thread of task #%d is dead",gtktask->ID);
  			gtk_list_store_set(GTK_LIST_STORE(model), &iter,0, out_str,-1);
  		}
	}
	task_to_kill=gtktask;
	gtktask=gtktask->next; if(gtktask==NULL) gtktask=primo_task;
	if(task_to_kill->cur_thread==NULL)
	{
		if(gtkhistory)
		{
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
			sprintf(out_str,"the task #%d is dead",task_to_kill->ID);
  			gtk_list_store_set(GTK_LIST_STORE(model), &iter,0, out_str,-1);
  		}
  		if(gtkwarrior)
  		{
  			
  		}
  		gtk_update_warrior(task_to_kill);
		del_task(task_to_kill);
	}
	return TRUE;
}
void multi_win_init()
{
	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window1), 1);
        gtk_window_set_title (GTK_WINDOW (window1), "WiCE");
        //gtk_window_set_policy(GTK_WINDOW(window1),FALSE,FALSE,FALSE);
        gtk_window_set_default_size (GTK_WINDOW (window1), max_x*6, max_y*6);
        //gtk_window_set_default_icon_from_file (PIXMAPS_DIR "/hello-icon.gif",NULL);
        g_signal_connect (G_OBJECT (window1), "destroy",G_CALLBACK (on_destroy), NULL);
	vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window1),vbox);
	gtk_widget_show(vbox);
	//add draw area
	drawing_area = gtk_drawing_area_new ();
    	gtk_widget_set_size_request (drawing_area, max_x*6, max_y*6);
    	gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",(GtkSignalFunc) main_expose, NULL);
    	gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event",(GtkSignalFunc) main_configure, NULL);   
    	gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK);
    	gtk_box_pack_start(GTK_BOX(vbox),drawing_area,TRUE,TRUE,0);
    	gtk_widget_show(drawing_area);
	statusbar=gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox),statusbar,FALSE,FALSE,0);
	gtk_widget_show(statusbar);
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar),"my_statusbar");
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, "Not Running.");
	
	gtk_widget_show(window1);
	
	window2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window2), 1);
        gtk_window_set_title (GTK_WINDOW (window2), "History");
        gtk_window_set_default_size (GTK_WINDOW (window2), 600, 200);
        g_signal_connect (G_OBJECT (window2), "destroy",G_CALLBACK (on_history_destroy), NULL);
        scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_container_add(GTK_CONTAINER(window2),scrolledwindow2);
  	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow2), 10);
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_widget_show (scrolledwindow2);
  	model     = gtk_list_store_new(1, G_TYPE_STRING);
  	view      = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
  	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),-1,"History",renderer,"text",0,NULL);
  	gtk_widget_show (view);
  	g_object_unref(model);
  	gtk_container_add(GTK_CONTAINER(scrolledwindow2),view);
  	gtk_window_move(GTK_WINDOW(window2),70,650);
        gtk_widget_show(window2);

	window3 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window3), 1);
        gtk_window_set_title (GTK_WINDOW (window3), "Warriors");
        gtk_window_set_default_size (GTK_WINDOW (window3), 200, 400);
        g_signal_connect (G_OBJECT (window3), "destroy",G_CALLBACK (on_warrior_destroy), NULL);
        scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_container_add(GTK_CONTAINER(window3),scrolledwindow3);
  	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow3), 10);
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_widget_show (scrolledwindow3);
  	model2     = gtk_list_store_new(3, GDK_TYPE_PIXBUF,G_TYPE_STRING,G_TYPE_STRING);
  	view2      = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model2));
  	selection2 = gtk_tree_view_get_selection(GTK_TREE_VIEW(view2));
	renderer = gtk_cell_renderer_pixbuf_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"<>",renderer,"pixbuf",0,NULL);
  	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"Threads",renderer,"text",1,NULL);
  	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"Warriors",renderer,"text",2,NULL);
  	gtk_widget_show (view2);
  	g_object_unref(model2);
  	gtk_container_add(GTK_CONTAINER(scrolledwindow3),view2);
  	gtk_window_move(GTK_WINDOW(window3),660,150);
        gtk_widget_show(window3);
}
void one_win_init()
{
	window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (window1), 1);
        gtk_window_set_title (GTK_WINDOW (window1), "WiCE");
        gtk_window_set_default_size (GTK_WINDOW (window1), max_x*6, max_y*6);
        //gtk_window_set_default_icon_from_file (PIXMAPS_DIR "/hello-icon.gif",NULL);
        g_signal_connect (G_OBJECT (window1), "destroy",G_CALLBACK (on_destroy), NULL);
	vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window1),vbox);
	gtk_widget_show(vbox);
	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);
	gtk_widget_show(hbox);
	scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_box_pack_start(GTK_BOX(hbox),scrolledwindow1,TRUE,TRUE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow1), 10);
  	gtk_widget_set_size_request(scrolledwindow1, max_x*7, max_y*7);
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_widget_show (scrolledwindow1);
	//area disegno
	drawing_area = gtk_drawing_area_new ();
    	gtk_widget_set_size_request (drawing_area, max_x*6, max_y*6);
    	gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",(GtkSignalFunc) main_expose, NULL);
    	gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event",(GtkSignalFunc) main_configure, NULL);   
    	gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK);
    	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolledwindow1),drawing_area);
    	gtk_widget_show(drawing_area);
	scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_box_pack_start(GTK_BOX(hbox),scrolledwindow3,FALSE,FALSE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow3), 10);
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_widget_show (scrolledwindow3);
  	model2     = gtk_list_store_new(3, GDK_TYPE_PIXBUF,G_TYPE_STRING,G_TYPE_STRING);
  	view2      = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model2));
  	selection2 = gtk_tree_view_get_selection(GTK_TREE_VIEW(view2));
	renderer = gtk_cell_renderer_pixbuf_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"<>",renderer,"pixbuf",0,NULL);
  	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"Threads",renderer,"text",1,NULL);
  	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view2),-1,"Warriors",renderer,"text",1,NULL);
  	gtk_widget_show (view2);
  	g_object_unref(model2);
  	gtk_widget_set_size_request(view2, 150, 200);
  	gtk_container_add(GTK_CONTAINER(scrolledwindow3),view2);
  	//gtk_window_move(GTK_WINDOW(window3),960,300);
        gtk_widget_show(window3);
	scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  	gtk_box_pack_start(GTK_BOX(vbox),scrolledwindow2,FALSE,FALSE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow2), 10);
  	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_widget_show (scrolledwindow2);
  	model     = gtk_list_store_new(1, G_TYPE_STRING);
  	view      = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
  	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    	renderer = gtk_cell_renderer_text_new();
  	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),-1,"History",renderer,"text",0,NULL);
  	gtk_widget_show (view);
  	g_object_unref(model);
  	gtk_widget_set_size_request(view, 100, 200);
  	gtk_container_add(GTK_CONTAINER(scrolledwindow2),view);
  	//gtk_window_move(GTK_WINDOW(window2),90,700);
        gtk_widget_show(window2);
	//gtk_signal_connect (GTK_OBJECT (view), "scroll_event",(GtkSignalFunc) my_get_scroll, NULL);
	statusbar=gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(vbox),statusbar,FALSE,FALSE,0);
	gtk_widget_show(statusbar);
	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar),"my_statusbar");
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, "Not Running.");
        gtk_widget_show(window1);
}
void init_x11()
{
	GtkWidget *label;
	struct process_task *ptask;
	guint32 m_RGBA;
	gtk_init(NULL,NULL);
	gtkhistory=1;
	gtkwarrior=1;
	gtk_sc_x=max_x;
	gtk_sc_y=max_y;
	if(b_multi_win)
	{
		multi_win_init();
	}
	else
	{
		one_win_init();
	}
	for(ptask=primo_task;ptask;ptask=ptask->next)
	{
		do {
		ptask->m_color.red = random() % 65535;
		ptask->m_color.green = random() % 65535;
		ptask->m_color.blue = random() % 65535;
		} while (ptask->m_color.red+ptask->m_color.green+ptask->m_color.blue<100000);
		gdk_color_alloc (execute_colormap, &(ptask->m_color));
	}
	ptask=primo_task;
	if(gtkwarrior)
	{
		do {
			gtk_list_store_append(GTK_LIST_STORE(model2), &iter);
			pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,TRUE,8,10,10);
			//color_icon(pixbuf,0,0,ptask->m_color.red,ptask->m_color.green,ptask->m_color.blue,1);
			//m_RGBA=get_RGBA(ptask->m_color.red,ptask->m_color.green,ptask->m_color.blue);
			//gdk_pixbuf_fill(pixbuf,m_RGBA);
			sprintf(out_str,"#%d (%c) color(%d,%d,%d)",ptask->ID,ptask->out_symbol,ptask->m_color.red,ptask->m_color.green,ptask->m_color.blue);
			bullet = gdk_pixmap_new (drawing_area->window, 10,10, -1);
  			gdk_gc_set_foreground (execute_gc, &(ptask->m_color));
  			gdk_draw_rectangle (bullet, execute_gc, TRUE, 0, 0,10,10);
  			gdk_pixbuf_get_from_drawable(pixbuf,bullet,NULL,0,0,0,0,10,10);
  			gtk_list_store_set(GTK_LIST_STORE(model2), &iter,0, pixbuf,1,"1",2,out_str,-1);
  			g_object_unref(bullet);
  			g_object_unref(pixbuf);
  			ptask=ptask->next;
  		}while(ptask);
  	}
  	// set gtk_timeout_add
  	timeout_tag=gtk_timeout_add(gtkwaittime,gtk_execute,NULL);
  	//timeout_tag=gtk_timeout_add(STAT_WAIT_TIME,gtk_statistic,NULL);
  	context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR (statusbar),"my_statusbar");
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, "Running....");
	gtktask=get_first();
        gtk_main ();

}
void deinit_x11()
{
	//gtk_exit(0);
}
void x11_print_arena_snap(void)
{

}
