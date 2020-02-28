//
// Created by salvo on 26/02/20.
//

#include "UI.h"

static pngImage image;
static double wnd_w, wnd_h=1000;

static void clear(cairo_t *cr, int xpoints, int ypoints){
    cairo_set_source_rgb(cr, .3, 0, .25);
    cairo_paint(cr);

    xpoints = 20.0, ypoints = 20.0;
    cairo_scale(cr, wnd_w/xpoints, wnd_h/ypoints);
    for(int i=0; i<xpoints; i++){
        for(int j=(i%2==0?0:1); j<ypoints; j+=2){
            cairo_rectangle(cr, (double) j, i, 1, 1);
        }
    }
    cairo_set_source_rgb(cr, 0, .25, .25);
    //cairo_set_source_rgb(cr, .5, .5, .5);
    cairo_fill(cr);
}

static void drawImage(cairo_t  *cr){
    pixel **matrix = image->image;
    float w = image->w, h = image->h;
    double r, g, b, a;
    cairo_scale(cr, wnd_w/w, wnd_h/h);
    for(int y=0; y<h; y++){
        for(int x=0; x<w; x++){
            pixel p = matrix[y][x];
            r = p.r / 255.0;
            g = p.g / 255.0;
            b = p.b / 255.0 ;
            //a = 1;
            a = p.a / 255.0;
            cairo_set_source_rgba(cr, r,g,b,a);
            //cairo_rectangle(cr, x-0.05, y-0.05, 1.1, 1.1);
            cairo_rectangle(cr, x, y, 1, 1);
            cairo_fill(cr);
        }
    }
}

static gboolean draw(GtkWidget *widget, cairo_t *cr, gpointer data){
    cairo_surface_t *s;
    s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, wnd_w, wnd_h);
    cairo_set_source_surface(cr, s, 0, 0);

    //clear(cr, 20, 20);
    drawImage(cr);
}

static void windowSetup(GtkApplication *app, gpointer user_data){
    GtkWindow *window;
    GtkWidget *frame, *drawingArea;

    wnd_w = (double) image->w / (double) image->h * wnd_h;
    //wnd_h = image->h / image->w * wnd_w;
    printf("WND_W: %lf\n", wnd_w);

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