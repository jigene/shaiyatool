///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef STDAFX_H
#define STDAFX_H

#define APP_NAME	"ModelEditor"
#define MODEL_EDITOR

class CImporter;
class CMainFrame;
class CExporter;
class CDialogEditEffects;
class CDialogSkinAuto;
class CModelViewer;

#define MODEL_EDITOR_FRIENDS	friend class CImporter; friend class CMainFrame; friend class CExporter; friend class CDialogEditEffects; friend class CDialogSkinAuto; friend class CModelViewer;

#include <Common.h>

#include <QtXML\qdom.h>
#include <QtMultimedia\QSoundEffect.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#endif // STDAFX_H