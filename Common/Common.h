///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef COMMON_H
#define COMMON_H

#define VERSION		1
#define SUB_VERSION	2

// pour les defines en .h
// détermine la version du GUIEditor aussi
#define __VER	15

#define QT_MESSAGELOGCONTEXT

#include <QtCore/qfile.h>
#include <QtCore/qmath.h>
#include <QtCore/qdir.h>
#include <QtCore/qprocess.h>
#include <QtCore/qsettings.h>
#include <QtCore/qstringlistmodel.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qelapsedtimer.h>
#include <QtCore/qtranslator.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qtimer.h>
#include <QtCore/QMimeData>
#include <QtGui/qstandarditemmodel.h>
#include <QtGui/qpainter.h>
#include <QtGui/QMouseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qmainwindow.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qinputdialog.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qsplashscreen.h>
#include <QtWidgets/qtextedit.h>
#include <QtWidgets/qtreeview.h>
#include <QtWidgets/qcolordialog.h>
#include <QtWidgets/qcombobox.h>
#include <QtWidgets/qaction.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/QUndoCommand>
#include <d3dx9.h>

#ifdef GetObject
#undef GetObject
#endif // GetObject

#define null 0

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef QString string;
typedef uint64_t objid;

class CTexture;
class CMotion;
class CSkeleton;
class CObject3D;
class CSfx;
class CTextFile;

struct Bounds
{
	D3DXVECTOR3 Min;
	D3DXVECTOR3 Max;
};

template<class T> void Delete(T*& val)
{
	delete val;
	val = null;
}

template<class T> void DeleteArray(T*& val)
{
	delete[] val;
	val = null;
}

template<class T> void Release(T*& val)
{
	if (val)
	{
		val->Release();
		val = null;
	}
}

void InstallMsgHandler();

string GetExtension(const string& filename);

float RoundFloat(float value, float round);

#include "PtrArray.h"
#include "File.h"
#include <define.h>
#include <defineNeuz.h>
#include <lang.h>

#endif // COMMON_H