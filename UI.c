//
// Created by salvo on 26/02/20.
//

#include "UI.h"

//#define DEBUG

#define m 434e-9
#define c 1.1

static pngImage image;
static double wnd_w, wnd_h;
double w,h;

static void clear(cairo_t *cr, int xpoints, int ypoints){
    //cairo_set_source_rgb(cr, .3, 0, .25);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    int l = w>h?w:h;

    for(int i=0; i<xpoints; i++){
        for(int j=(i%2==0?0:1); j<ypoints; j+=2){
            cairo_rectangle(cr, (double) j*l/xpoints, i*l/ypoints, l/xpoints, l/xpoints);
        }
    }
    //cairo_set_source_rgb(cr, 0, .25, .25);
    cairo_set_source_rgb(cr, .5, .5, .5);
    cairo_fill(cr);
}

static void drawImage(cairo_t  *cr){
    pixel **matrix = image->image;
    double r, g, b, a;

    cairo_text_extents_t te;
    cairo_select_font_face (cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    // m and c found with a lienear intererpolation knowing good results
    // for (50x28) -> 1.1 and (1920x1080) -> 2
    // giver (w,h) -> a
    // where w,h are width and height of an image
    // and a is the pixel size
    double pixelSize=w*h*m+c, pixelOffset=(pixelSize-1)/2;
#ifdef DEBUG
    printf("PixelSize: %lf\n", pixelSize);
#endif

    //pixelSize = 100;
    for(int y=0; y<h; y++) {
        //printf("line: %d\n", y);
        for (int x = 0; x<w; x++) {
            pixel p = matrix[y][x];
            r = p.r / 255.0;
            g = p.g / 255.0;
            b = p.b / 255.0;
            a = p.a / 255.0;
            cairo_set_source_rgba(cr, r, g, b, a);
#ifdef DEBUG
            printf("%d %d %3d %3d %3d %3d\n", y, x, p.r, p.g, p.b, p.a);
#endif
            cairo_rectangle(cr, x-pixelOffset, y-pixelOffset, pixelSize, pixelSize);
            cairo_fill(cr);
        }
    }
}

static gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data){
    cairo_surface_t *s;
    s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, wnd_w, wnd_h);
    cairo_set_source_surface(cr, s, 0, 0);
    w = image->w, h = image->h;
    cairo_scale(cr, wnd_w/w, wnd_h/h);

    clear(cr, 20, 20);
    drawImage(cr);
}

static void windowSetup(GtkApplication *app, gpointer user_data){
    GtkWindow *window;
    GtkWidget *frame, *drawingArea;


    if(image->w > image->h){
        wnd_h = 750;
        wnd_w = (double) image->w / (double) image->h * wnd_h;
    }else{
        wnd_w = 500;
        wnd_h = (double) image->h / (double) image->w * wnd_w;
    }

#ifdef DEBUG
    printf("WND_W: %lf\n", wnd_w);
#endif

    window = GTK_WINDOW( gtk_application_window_new(app) );
    gtk_window_set_title(window, "pngDecoder");
    gtk_window_set_default_size(GTK_WINDOW(window), wnd_w, wnd_h);
    gtk_window_set_resizable(window, gtk_false());

    frame = gtk_frame_new(NULL);
    gtk_container_add(GTK_CONTAINER(window), frame);


    drawingArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(frame), drawingArea);
    g_signal_connect(drawingArea, "draw", G_CALLBACK(draw), NULL);

    gtk_widget_show_all(GTK_WIDGET(window));
}

int UI_displayImage(pngImage pi){
    GtkApplication *app;
    int status;

    image = pi;
    app = gtk_application_new("org.gtk.app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(windowSetup), NULL);
    status = g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);

    return status;
}