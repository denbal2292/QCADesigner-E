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
// This file was contributed by Gabriel Schulhof        //
// (schulhof@atips.ca).                                 //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Stuff that wouldn't fit anywhere else.               //
//                                                      //
//////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
  #include <unistd.h>
#endif /* ndef WIN32 */
#include <glib.h>
#include <glib/gstdio.h>
#include "exp_array.h"
#include "generic_utils.h"
#include "global_consts.h"

#ifdef GTK_GUI
typedef struct
  {
  char *pszCmdLine ;
  char *pszTmpFName ;
  } RUN_CMD_LINE_ASYNC_THREAD_PARAMS ;

static gpointer RunCmdLineAsyncThread (gpointer p) ;
#endif /* def GTK_GUI */

// Causes a rectangle of width (*pdRectWidth) and height (*pdRectHeight) to fit inside a rectangle of
// width dWidth and height dHeight.  Th resulting pair ((*px),(*py)) holds the coordinates of the
// upper left corner of the scaled rectangle wrt. the upper left corner of the given rectangle.
void fit_rect_inside_rect (double dWidth, double dHeight, double *px, double *py, double *pdRectWidth, double *pdRectHeight)
  {
  double dAspectRatio, dRectAspectRatio ;

  if (0 == dWidth || 0 == dHeight || 0 == *pdRectWidth || 0 == *pdRectHeight) return ;

  dAspectRatio = dWidth / dHeight ;
  dRectAspectRatio = *pdRectWidth / *pdRectHeight ;

  if (dRectAspectRatio > dAspectRatio)
    {
    *px = 0 ;
    *pdRectWidth = dWidth ;
    *pdRectHeight = *pdRectWidth / dRectAspectRatio ;
    *py = (dHeight - *pdRectHeight) / 2 ;
    }
  else
    {
    *py = 0 ;
    *pdRectHeight = dHeight ;
    *pdRectWidth = *pdRectHeight * dRectAspectRatio ;
    *px = (dWidth - *pdRectWidth) / 2 ;
    }
  }

// Convert a long long value to decimal, hexadecimal, or binary
char *strdup_convert_to_base (long long value, int base)
  {
  if (10 == base)
    return g_strdup_printf ("%llu", value) ;
  else
  if (16 == base)
    return g_strdup_printf ("%llX", value) ;
  else
  if (2 == base)
    {
    char *psz = NULL ;
    EXP_ARRAY *str = NULL ;

    if (0 == value)
      return g_strdup ("0") ;

    str = exp_array_new (sizeof (char), 1) ;

    while (value)
      {
      exp_array_insert_vals (str, NULL, 1, 1, -1) ;
      exp_array_index_1d (str, char, str->icUsed - 1) = (value & 0x1) ? '1' : '0' ;
      value = value >> 1 ;
      }

    exp_array_insert_vals (str, NULL, 1, 1, -1) ;
    exp_array_index_1d (str, char, str->icUsed - 1) = 0 ;
    psz = g_strreverse (g_strndup (str->data, str->icUsed)) ;
    exp_array_free (str) ;
    return psz ;
    }
  return NULL ;
  }

// Decide whether the slope difference between two line segments is significant enough for the three points
// demarcating the two line segments not to be considered collinear
//                     (dx2,dy2)
//                        /
//                       /
//(dx0,dy0)             /
//    _________________/
//                   (dx1,dy1)
gboolean LineSegmentCanBeSkipped (double dx0, double dy0, double dx1, double dy1, double dx2, double dy2, double dMaxSlopeDiff)
  {
  if (dx0 == dx1 && dx1 == dx2) return TRUE ;
  if (dy0 == dy1 && dy1 == dy2) return TRUE ;

  return (fabs ((dy1 - dy0) / (dx1 - dx0) - (dy2 - dy1) / (dx2 - dx1)) < dMaxSlopeDiff) ;
  }

// Converts text to a number with either a period or a comma as the decimal separator,
// whatever the locale. Design files saved on systems using commas contain them, and so
// can numbers typed into dialogs.
double qcad_strtod (const char *psz)
  {
  char *pszComma = NULL, *pszCopy = NULL ;
  double dValue = 0.0 ;

  if (NULL == psz) return 0.0 ;
  if (NULL == strchr (psz, ',')) return g_ascii_strtod (psz, NULL) ;

  pszCopy = g_strdup (psz) ;
  if (NULL != (pszComma = strchr (pszCopy, ',')))
    (*pszComma) = '.' ;
  dValue = g_ascii_strtod (pszCopy, NULL) ;
  g_free (pszCopy) ;

  return dValue ;
  }

// Returns the directory holding the installed data files. This is the "share" directory
// next to the program's "bin" directory if the package's files are there, so that the
// program can be moved after installing it. Otherwise, it is the one chosen by configure.
const char *get_package_data_dir ()
  {
  static char *pszDataDir = NULL ;

  if (NULL == pszDataDir)
    {
#ifdef WIN32
    // The installation directory is the one containing "bin", returned in UTF-8
    char *pszInstall = g_win32_get_package_installation_directory_of_module (NULL) ;

    if (NULL != pszInstall)
      {
      pszDataDir = g_build_filename (pszInstall, "share", NULL) ;
      g_free (pszInstall) ;
      }
#else
    char *pszExe = g_file_read_link ("/proc/self/exe", NULL) ;

    if (NULL != pszExe)
      {
      char *pszBinDir = g_path_get_dirname (pszExe) ;
      char *pszPrefix = g_path_get_dirname (pszBinDir) ;
      char *pszShare = g_build_filename (pszPrefix, "share", NULL) ;
      char *pszPackage = g_build_filename (pszShare, PACKAGE, NULL) ;

      if (g_file_test (pszPackage, G_FILE_TEST_IS_DIR))
        pszDataDir = g_strdup (pszShare) ;

      g_free (pszPackage) ;
      g_free (pszShare) ;
      g_free (pszPrefix) ;
      g_free (pszBinDir) ;
      g_free (pszExe) ;
      }
#endif /* def WIN32 */

    if (NULL == pszDataDir)
      pszDataDir = g_strdup (PACKAGE_DATA_DIR) ;
    }

  return pszDataDir ;
  }

#ifdef GTK_GUI
void RunCmdLineAsync (char *pszCmdLine, char *pszTmpFName)
  {
  RUN_CMD_LINE_ASYNC_THREAD_PARAMS *prclap = g_malloc0 (sizeof (RUN_CMD_LINE_ASYNC_THREAD_PARAMS)) ;

  prclap->pszCmdLine = g_strdup (pszCmdLine) ;
  prclap->pszTmpFName = (NULL == pszTmpFName ? NULL : g_strdup (pszTmpFName)) ;

  if (!g_thread_supported ()) g_thread_init (NULL) ;

  g_thread_create ((GThreadFunc)RunCmdLineAsyncThread, (gpointer)prclap, FALSE, NULL) ;
  }

static gpointer RunCmdLineAsyncThread (gpointer p)
  {
  RUN_CMD_LINE_ASYNC_THREAD_PARAMS *prclap = (RUN_CMD_LINE_ASYNC_THREAD_PARAMS *)p ;
#ifdef WIN32
  STARTUPINFOW si ;
  PROCESS_INFORMATION pi ;
  // The command line is UTF-8, so start the program through the Unicode API
  wchar_t *pwszCmdLine = (wchar_t *)g_utf8_to_utf16 (prclap->pszCmdLine, -1, NULL, NULL, NULL) ;

  memset (&si, 0, sizeof (si)) ;
  memset (&pi, 0, sizeof (pi)) ;
  si.cb = sizeof (STARTUPINFOW) ;

  if (NULL != pwszCmdLine && CreateProcessW (NULL, pwszCmdLine, NULL, NULL, FALSE, DETACHED_PROCESS,
    NULL, NULL, &si, &pi))
    {
    WaitForSingleObject (pi.hProcess, INFINITE) ;
    CloseHandle (pi.hProcess) ;
    CloseHandle (pi.hThread) ;
    }
  g_free (pwszCmdLine) ;
#else
  // Inside an AppImage, other programs must not inherit the settings for its bundled libraries
  if (NULL != getenv ("APPDIR"))
    {
    char *pszCmdLine = g_strdup_printf ("env -u LD_LIBRARY_PATH -u GTK2_RC_FILES -u GDK_PIXBUF_MODULE_FILE -u GTK_IM_MODULE_FILE %s", prclap->pszCmdLine) ;

    system (pszCmdLine) ;
    g_free (pszCmdLine) ;
    }
  else
    system (prclap->pszCmdLine) ;
#endif
  g_free (prclap->pszCmdLine) ;
  if (NULL != prclap->pszTmpFName)
    {
    g_unlink (prclap->pszTmpFName) ;
    g_free (prclap->pszTmpFName) ;
    }
  g_free (prclap) ;

  return NULL ;
  }
#endif /* def GTK_GUI */

char *get_enum_string_from_value (GType enum_type, int value)
  {
  GEnumClass *klass = g_type_class_ref (enum_type) ;
  GEnumValue *val = NULL ;

  if (NULL == klass) return g_strdup_printf ("%d", value) ;

  if (NULL == (val = g_enum_get_value (klass, value)))
    return g_strdup_printf ("%d", value) ;
  else
    return g_strdup (val->value_name) ;

  g_type_class_unref (klass) ;
  }

int get_enum_value_from_string (GType enum_type, char *psz)
  {
  GEnumClass *klass = g_type_class_peek (enum_type) ;
  GEnumValue *val = NULL ;

  if (NULL == klass) return g_ascii_strtod (psz, NULL) ;

  if (NULL == (val = g_enum_get_value_by_name (klass, psz)))
    return g_ascii_strtod (psz, NULL) ;
  else
    return val->value ;
  }
