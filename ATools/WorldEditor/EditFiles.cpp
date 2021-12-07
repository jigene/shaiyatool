///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "MainFrame.h"
#include "WorldEditor.h"
#include <ModelMng.h>
#include <World.h>
#include "Navigator.h"
#include "NewWorld.h"
#include "WorldProperties.h"
#include "ContinentEdit.h"
#include <Skybox.h>
#include <GameElements.h>
#include <Project.h>
#include <Object.h>

void CMainFrame::_editModel(QStandardItem* element)
{
	if (!element)
		return;

	if (element->type() == GAMEELE_MODEL)
	{
		CModelElement* modelElement = (CModelElement*)element;
		ModelProp* prop = modelElement->GetModel();

		if (!prop)
			return;

		QProcess* process = new QProcess(this);
		if (prop->modelType == MODELTYPE_SFX)
			process->start("SfxEditor.exe", QStringList("SFX/" + CObject::GetModelFilename(prop)));
		else
		{
			QStringList args("Model/" + CObject::GetModelFilename(prop));
			if (prop->textureEx != ATEX_00)
				args.append(string::number(prop->textureEx));
			process->start("ModelEditor.exe", args);
		}
	}
}

void CMainFrame::HideDialogs()
{
	if (m_editor)
		m_editor->MouseLost();
	if (m_dialogWorldProperties)
		m_dialogWorldProperties->hide();
	if (m_dialogContinentEdit)
		m_dialogContinentEdit->hide();
}

void CMainFrame::ShowDialogs()
{
	if (m_dialogWorldProperties)
		m_dialogWorldProperties->show();
	if (m_dialogContinentEdit)
		m_dialogContinentEdit->show();
}

void CMainFrame::EditWorldProperties()
{
	if (!m_world)
		return;

	if (m_dialogWorldProperties)
		Delete(m_dialogWorldProperties);
	else
	{
		m_dialogWorldProperties = new CDialogWorldProperties(m_world, this);
		connect(m_dialogWorldProperties, SIGNAL(finished(int)), this, SLOT(DialogWorldPropertiesClosed(int)));
		m_dialogWorldProperties->show();
	}
}

void CMainFrame::DialogWorldPropertiesClosed(int result)
{
	m_dialogWorldProperties = null;
	ui.actionPropri_t_s_du_monde->setChecked(false);
}

void CMainFrame::EditContinents()
{
	if (!m_world)
		return;

	if (m_dialogContinentEdit)
	{
		Delete(m_dialogContinentEdit);
		DialogEditContinentsClosed(0);
	}
	else
	{
		m_dialogContinentEdit = new CDialogContinentEdit(m_world, this);
		connect(m_dialogContinentEdit, SIGNAL(finished(int)), this, SLOT(DialogEditContinentsClosed(int)));
		m_dialogContinentEdit->show();
		g_global3D.continentVertices = true;
	}
}

void CMainFrame::DialogEditContinentsClosed(int result)
{
	m_dialogContinentEdit = null;
	ui.action_dition_des_continents->setChecked(false);
	g_global3D.continentVertices = false;
	if (m_world)
	{
		m_world->UpdateContinent();
		UpdateWorldEditor();
	}
}

void CMainFrame::UpdateContinentVertices()
{
	if (m_dialogContinentEdit)
		m_dialogContinentEdit->UpdateVertexList();
}

void CMainFrame::OpenFile(const string& filename)
{
	CloseFile();

	m_world = new CWorld(m_editor->GetDevice());
	if (m_world->Load(filename))
	{
		m_filename = filename;
		setWindowTitle("WorldEditor - " + QFileInfo(filename).fileName());

		m_lastOpenFilenames.removeAll(filename);
		m_lastOpenFilenames.push_front(filename);
		_updateLastOpenFiles();
		UpdatePatrolList();
	}
	else
		Delete(m_world);

	m_navigator->SetWorld(m_world);
	m_editor->SetWorld(m_world);
}

void CMainFrame::OpenFile()
{
	HideDialogs();

	if (m_world && !m_undoStack->isClean())
	{
		QMessageBox::Button result = QMessageBox::question(this, tr("Attention"), tr("Êtes vous sûr de vouloir fermer la map sans l'avoir sauvegardé ?"));
		if (result != QMessageBox::Yes)
		{
			ShowDialogs();
			return;
		}
	}

	const string filename = QFileDialog::getOpenFileName(this, tr("Charger une map"), m_filename.isEmpty() ? "World/" : m_filename, tr("Fichier world") + " (*.wld)");

	ShowDialogs();

	if (!filename.isEmpty())
	{
		m_undoStack->clear();
		OpenFile(filename);
	}
}

void CMainFrame::CloseFile()
{
	if (m_world && !m_undoStack->isClean())
	{
		HideDialogs();
		QMessageBox::Button result = QMessageBox::question(this, tr("Attention"), tr("Êtes vous sûr de vouloir fermer la map sans l'avoir sauvegardé ?"));
		if (result != QMessageBox::Yes)
		{
			ShowDialogs();
			return;
		}
		ShowDialogs();
		m_undoStack->clear();
	}

	Delete(m_world);
	Delete(m_dialogWorldProperties);
	Delete(m_dialogContinentEdit);
	ui.actionPropri_t_s_du_monde->setChecked(false);
	ui.action_dition_des_continents->setChecked(false);
	setWindowTitle("WorldEditor");
	m_filename.clear();
	if (m_navigator)
		m_navigator->SetWorld(m_world);
	if (m_editor)
		m_editor->SetWorld(m_world);
	SetStatusBarInfo(D3DXVECTOR3(0, 0, 0), 0, 0, 0, "", "");
	SetLayerInfos(null);
	m_currentPatrol = -1;
	UpdatePatrolList();
}

void CMainFrame::_updateLastOpenFiles()
{
	ui.menuFichiers_r_cents->clear();
	m_lastOpenFilesActions.clear();

	while (m_lastOpenFilenames.size() > 10)
		m_lastOpenFilenames.pop_back();

	QAction* action;
	for (int i = 0; i < m_lastOpenFilenames.size(); i++)
	{
		action = ui.menuFichiers_r_cents->addAction(string::number(i + 1) + ". " + QFileInfo(m_lastOpenFilenames[i]).fileName());
		m_lastOpenFilesActions[action] = m_lastOpenFilenames[i];
	}
}

void CMainFrame::OpenLastFile(QAction* action)
{
	if (m_world && !m_undoStack->isClean())
	{
		HideDialogs();
		QMessageBox::Button result = QMessageBox::question(this, tr("Attention"), tr("Êtes vous sûr de vouloir fermer la map sans l'avoir sauvegardé ?"));
		if (result != QMessageBox::Yes)
		{
			ShowDialogs();
			return;
		}
		ShowDialogs();
	}

	auto it = m_lastOpenFilesActions.find(action);
	if (it != m_lastOpenFilesActions.end())
	{
		m_undoStack->clear();
		OpenFile(it.value());
	}
}

void CMainFrame::NewFile()
{
	HideDialogs();

	if (m_world && !m_undoStack->isClean())
	{
		QMessageBox::Button result = QMessageBox::question(this, tr("Attention"), tr("Êtes vous sûr de vouloir fermer la map sans l'avoir sauvegardé ?"));
		if (result != QMessageBox::Yes)
		{
			ShowDialogs();
			return;
		}
	}

	CDialogNewWorld dialog(this);
	if (dialog.exec() == QDialog::Accepted)
	{
		m_undoStack->clear();
		CloseFile();
		m_world = new CWorld(m_editor->GetDevice());
		dialog.CreateWorld(m_world);
		setWindowTitle("WorldEditor - " + tr("Nouvelle map"));
		m_navigator->SetWorld(m_world);
		m_editor->SetWorld(m_world);

		ui.action_dition_des_continents->setChecked(false);
		ui.action_dition_des_continents->setChecked(false);
	}
	ShowDialogs();
}

void CMainFrame::SaveBigmap()
{
	if (!m_world)
		return;

	HideDialogs();
	const string filename = QFileDialog::getSaveFileName(this, tr("Enregistrer l'image"), "", tr("Fichier image") + " (*.png *.bmp)");

	if (!filename.isEmpty() && m_world->m_width * m_world->m_height > 100 && !m_filename.isEmpty())
	{
		if (QMessageBox::question(this, tr("Attention"), tr("Cet map est très grande et son rendu complet va prendre plusieurs minutes.\n" \
			"Afin d'éviter une surconsommation de mémoire, les fichiers vont être chargés et déchargés plusieurs fois.\n" \
			"Le travail non enregistré sera perdu. Souhaitez-vous enregistrer votre travail ?")) == QMessageBox::Yes)
			m_world->Save(m_filename);
	}
	ShowDialogs();

	if (!filename.isEmpty())
		m_editor->SaveBigmap(filename);
}

void CMainFrame::SaveMinimaps()
{
	if (!m_world)
		return;

	if (m_filename.isEmpty())
	{
		HideDialogs();
		m_filename = QFileDialog::getSaveFileName(this, tr("Enregistrer la map"), "World/", tr("Fichier world") + " (*.wld)");
		ShowDialogs();

		if (!m_filename.isEmpty())
		{
			setWindowTitle("WorldEditor - " + QFileInfo(m_filename).fileName());

			m_lastOpenFilenames.removeAll(m_filename);
			m_lastOpenFilenames.push_front(m_filename);
			_updateLastOpenFiles();

			m_world->Save(m_filename);
		}
	}

	if (!m_filename.isEmpty())
		m_editor->SaveMinimaps();
}

void CMainFrame::SaveFile()
{
	if (!m_world)
		return;

	if (m_filename.isEmpty())
	{
		HideDialogs();
		m_filename = QFileDialog::getSaveFileName(this, tr("Enregistrer la map"), "World/", tr("Fichier world") + " (*.wld)");
		ShowDialogs();

		if (!m_filename.isEmpty())
		{
			setWindowTitle("WorldEditor - " + QFileInfo(m_filename).fileName());

			m_lastOpenFilenames.removeAll(m_filename);
			m_lastOpenFilenames.push_front(m_filename);
			_updateLastOpenFiles();
		}
	}

	if (!m_filename.isEmpty())
	{
		m_world->Save(m_filename);
		m_undoStack->setClean();
	}
}

void CMainFrame::SaveFileAs()
{
	if (!m_world)
		return;

	HideDialogs();
	const string filename = QFileDialog::getSaveFileName(this, tr("Enregistrer la map"), m_filename.isEmpty() ? "World/" : m_filename, tr("Fichier world") + " (*.wld)");
	ShowDialogs();

	if (!filename.isEmpty())
	{
		m_filename = filename;
		setWindowTitle("WorldEditor - " + QFileInfo(m_filename).fileName());
		m_lastOpenFilenames.removeAll(m_filename);
		m_lastOpenFilenames.push_front(m_filename);
		_updateLastOpenFiles();

		m_world->Save(m_filename);
		m_undoStack->setClean();
	}
}

void CMainFrame::dragEnterEvent(QDragEnterEvent* event)
{
	m_dragFilename.clear();

	const QMimeData* mimeData = event->mimeData();

	if (mimeData->hasUrls())
	{
		QStringList pathList;
		QList<QUrl> urlList = mimeData->urls();

		for (int i = 0; i < urlList.size() && i < 32; ++i)
			pathList.append(urlList.at(i).toLocalFile());

		if (pathList.size() > 0)
		{
			for (int i = 0; i < pathList.size(); i++)
			{
				if (GetExtension(pathList[i]) == "wld")
				{
					m_dragFilename = pathList[i];
					event->acceptProposedAction();
					return;
				}
			}

		}
	}

	m_dragFilename.clear();
}

void CMainFrame::dragLeaveEvent(QDragLeaveEvent* event)
{
	event->accept();
}

void CMainFrame::dragMoveEvent(QDragMoveEvent* event)
{
	if (m_dragFilename.size() > 0)
		event->acceptProposedAction();
}

void CMainFrame::dropEvent(QDropEvent* event)
{
	if (m_dragFilename.size() > 0)
		OpenFile(m_dragFilename);
}

void CMainFrame::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::WindowStateChange && !isMinimized())
	{
		if (m_editor && !m_editor->IsAutoRefresh())
			m_editor->RenderEnvironment();
	}

	QMainWindow::changeEvent(event);
}