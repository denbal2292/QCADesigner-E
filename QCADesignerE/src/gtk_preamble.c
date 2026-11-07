//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// GTK preamble. This is where we call gtk_init,        //
// register all the stock items, add the pixmap         //
// directories.                                         //
//                                                      //
//////////////////////////////////////////////////////////

#ifdef GTK_GUI

#include <locale.h>
#include "support.h"
#include "gtk_preamble.h"
#include "qcadstock.h"
#include "global_consts.h"
#include "custom_widgets.h"
#include "generic_utils.h"

#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
static void my_logger (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data) ;
#endif /* ifdef QCAD_NO_CONSOLE */
#endif /* ifdef WIN32 */

#ifdef QCAD_NO_CONSOLE
void gtk_preamble (int *pargc, char ***pargv, char *pszBaseName, char *pszCmdLine)
#else
void gtk_preamble (int *pargc, char ***pargv, char *pszBaseName)
#endif /* def QCAD_NO_CONSOLE */
  {
#ifdef QCAD_NO_CONSOLE
  // WinMain gets the command line in the system code page, which cannot hold every file
  // name, so take the arguments from Windows' Unicode command line instead, as UTF-8
  (*pargv) = g_win32_get_command_line () ;
  (*pargc) = g_strv_length (*pargv) ;
#endif /* def QCAD_NO_CONSOLE */

  gtk_init (pargc, pargv) ;

  // Set the locale
#ifdef ENABLE_NLS
  {
  char *pszLocaleDir = g_build_filename (get_package_data_dir (), "locale", NULL) ;

#ifdef WIN32
  // bindtextdomain takes a path in the system code page, which cannot hold every folder name
  wchar_t *pwszLocaleDir = (wchar_t *)g_utf8_to_utf16 (pszLocaleDir, -1, NULL, NULL, NULL) ;

  if (NULL != pwszLocaleDir)
    wbindtextdomain (PACKAGE, pwszLocaleDir) ;
  g_free (pwszLocaleDir) ;
#else
  bindtextdomain (PACKAGE, pszLocaleDir) ;
#endif /* def WIN32 */
  g_free (pszLocaleDir) ;
  }
  textdomain (PACKAGE);
#endif
  gtk_set_locale ();
  // gtk_init and gtk_set_locale adopt the system locale, which can make every number the
  // program formats use a comma. Files must always use periods, so keep them for numbers.
  setlocale (LC_NUMERIC, "C") ;

  // Add pixmap directories
  // -- Pixmaps used by the buttons in the main window -- //
  add_pixmap_directory (PACKAGE_DATA_DIR "/QCADesigner/pixmaps");
  add_pixmap_directory (PACKAGE_SOURCE_DIR "/pixmaps");
  // Directories added later are searched first. This path must be absolute, because
  // gtk_icon_source_set_filename rejects relative ones, and get_package_data_dir's is.
  {
  char *pszPixmapDir = g_build_filename (get_package_data_dir (), "QCADesigner", "pixmaps", NULL) ;

  add_pixmap_directory (pszPixmapDir) ;
  g_free (pszPixmapDir) ;
  }
// Done adding pixmap directories

#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
  // Turn off logging by setting it to an empty function
  // This prevents a console from popping up when QCADesigner quits
  g_log_set_handler (NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Gtk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Gdk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("GdkPixbuf", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
  g_log_set_handler ("Pango", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, my_logger, NULL);
#endif /* ifdef QCAD_NO_CONSOLE */
#endif /* ifdef WIN32 */

  set_window_icon (NULL, pszBaseName) ;

  add_stock_icon ("drawing_layer.png",      QCAD_STOCK_DRAWING_LAYER) ;
  add_stock_icon ("clocks_layer.png",       QCAD_STOCK_CLOCKING_LAYER) ;
  add_stock_icon ("cells_layer.png",        QCAD_STOCK_CELL_LAYER) ;
  add_stock_icon ("substrate_layer.png",    QCAD_STOCK_SUBSTRATE_LAYER) ;
  add_stock_icon ("label.png",              QCAD_STOCK_LABEL) ;
  add_stock_icon ("substrate.png",          QCAD_STOCK_SUBSTRATE) ;
#ifdef HAVE_LIBRSVG
  add_stock_icon ("reorder_layers.svg",       QCAD_STOCK_REORDER_LAYERS) ;
  add_stock_icon ("bus.svg",                  QCAD_STOCK_BUS) ;
  add_stock_icon ("clock.svg",                QCAD_STOCK_CLOCK) ;
  add_stock_icon ("bus_input.svg",            QCAD_STOCK_BUS_INPUT) ;
  add_stock_icon ("bus_output.svg",           QCAD_STOCK_BUS_OUTPUT) ;
  add_stock_icon ("cell_input.svg",           QCAD_STOCK_CELL_INPUT) ;
  add_stock_icon ("cell_output.svg",          QCAD_STOCK_CELL_OUTPUT) ;
  add_stock_icon ("graph_data_bin.svg",       QCAD_STOCK_GRAPH_BIN) ;
  add_stock_icon ("graph_data_hex.svg",       QCAD_STOCK_GRAPH_HEX) ;
  add_stock_icon ("graph_data_dec.svg",       QCAD_STOCK_GRAPH_DEC) ;
  add_stock_icon ("puzzle_piece_green.svg",   QCAD_STOCK_BLOCK_READ) ;
  add_stock_icon ("puzzle_piece_red.svg",     QCAD_STOCK_BLOCK_WRITE) ;
  add_stock_icon ("q_cell_def.svg",           QCAD_STOCK_CELL) ;
  add_stock_icon ("q_cell_array.svg",         QCAD_STOCK_ARRAY) ;
  add_stock_icon ("q_cell_rotate.svg",        QCAD_STOCK_ROTATE_CELL) ;
  add_stock_icon ("default.svg",              QCAD_STOCK_SELECT) ;
  add_stock_icon ("q_cell_alt.svg",           QCAD_STOCK_CELL_ALT_CROSSOVER) ;
  add_stock_icon ("q_cell_alt_circle.svg",    QCAD_STOCK_CELL_ALT_VERTICAL) ;
  add_stock_icon ("q_cell_copy.svg",          QCAD_STOCK_COPY) ;
  add_stock_icon ("q_cell_translate.svg",     QCAD_STOCK_TRANSLATE) ;
  add_stock_icon ("q_cell_mirror.svg",        QCAD_STOCK_MIRROR_VERTICAL) ;
  add_stock_icon ("q_cell_mirror_other.svg",  QCAD_STOCK_MIRROR_HORIZONTAL) ;
  add_stock_icon ("q_cell_pan.svg",           QCAD_STOCK_PAN) ;
  add_stock_icon ("ruler.svg",                QCAD_STOCK_MEASURE) ;
  add_stock_icon ("insert_column_before.svg", QCAD_STOCK_INSERT_COL_BEFORE) ;
#else
  add_stock_icon ("reorder_layers.png",       QCAD_STOCK_REORDER_LAYERS) ;
  add_stock_icon ("bus.png",                  QCAD_STOCK_BUS) ;
  add_stock_icon ("clock.png",                QCAD_STOCK_CLOCK) ;
  add_stock_icon ("bus_input.png",            QCAD_STOCK_BUS_INPUT) ;
  add_stock_icon ("bus_output.png",           QCAD_STOCK_BUS_OUTPUT) ;
  add_stock_icon ("cell_input.png",           QCAD_STOCK_CELL_INPUT) ;
  add_stock_icon ("cell_output.png",          QCAD_STOCK_CELL_OUTPUT) ;
  add_stock_icon ("graph_data_bin.png",       QCAD_STOCK_GRAPH_BIN) ;
  add_stock_icon ("graph_data_hex.png",       QCAD_STOCK_GRAPH_HEX) ;
  add_stock_icon ("graph_data_dec.png",       QCAD_STOCK_GRAPH_DEC) ;
  add_stock_icon ("puzzle_piece_green.png",   QCAD_STOCK_BLOCK_READ) ;
  add_stock_icon ("puzzle_piece_red.png",     QCAD_STOCK_BLOCK_WRITE) ;
  add_stock_icon ("q_cell_def.png",           QCAD_STOCK_CELL) ;
  add_stock_icon ("q_cell_array.png",         QCAD_STOCK_ARRAY) ;
  add_stock_icon ("q_cell_rotate.png",        QCAD_STOCK_ROTATE_CELL) ;
  add_stock_icon ("default.png",              QCAD_STOCK_SELECT) ;
  add_stock_icon ("q_cell_alt.png",           QCAD_STOCK_CELL_ALT_CROSSOVER) ;
  add_stock_icon ("q_cell_alt_circle.png",    QCAD_STOCK_CELL_ALT_VERTICAL) ;
  add_stock_icon ("q_cell_move.png",          QCAD_STOCK_COPY) ;
  add_stock_icon ("q_cell_move.png",          QCAD_STOCK_TRANSLATE) ;
  add_stock_icon ("q_cell_mirror.png",        QCAD_STOCK_MIRROR_VERTICAL) ;
  add_stock_icon ("q_cell_mirror_other.png",  QCAD_STOCK_MIRROR_HORIZONTAL) ;
  add_stock_icon ("q_cell_pan.png",           QCAD_STOCK_PAN) ;
  add_stock_icon ("ruler.png",                QCAD_STOCK_MEASURE) ;
  add_stock_icon ("insert_column_before.png", QCAD_STOCK_INSERT_COL_BEFORE) ;
#endif
  }

#ifdef QCAD_NO_CONSOLE
static void my_logger (const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
  {
  // Handle log messages here
  // This logger ignores all messages, so as not to produce a console window
  }
#endif /* ifdef QCAD_NO_CONSOLE */

#ifdef WIN32
#ifdef QCAD_NO_CONSOLE
#endif /* QCAD_NO_CONSOLE */
#endif /* ifdef WIN32 */

#endif /* def GTK_GUI */
