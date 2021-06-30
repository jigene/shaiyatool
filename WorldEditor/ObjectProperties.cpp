///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#include "stdafx.h"
#include "ObjectProperties.h"
#include <Mover.h>
#include <Region.h>
#include <World.h>
#include <ModelMng.h>
#include "MainFrame.h"
#include <TextFile.h>
#include <Project.h>
#include <Ctrl.h>
#include <Path.h>
#include <World.h>

CObjectPropertiesDialog::CObjectPropertiesDialog(CObject* obj, QWidget* parent)
	: QDialog(parent),
	m_obj(null)
{
	ui.setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVector<int> enabledTabs;
	switch (obj->m_type)
	{
	case OT_MOVER:
		enabledTabs.append(0);
		enabledTabs.append(2);
		break;
	case OT_ITEM:
		enabledTabs.append(0);
		break;
	case OT_CTRL:
		enabledTabs.append(0);
		enabledTabs.append(3);
		break;
	case OT_REGION:
		switch (obj->m_modelID)
		{
		case RI_STRUCTURE:
			enabledTabs.append(4);
			break;
		case RI_REVIVAL:
		case RI_PLACE:
			enabledTabs.append(1);
			break;
		case RI_ATTRIBUTE:
		case RI_TRIGGER:
			enabledTabs.append(5);
			break;
		}
		break;
	case OT_PATH:
		enabledTabs.append(6);
		break;
	}

	const int count = ui.tabWidget->tabBar()->count();
	for (int i = 0, j = 0; i < count; i++, j++)
	{
		if (!enabledTabs.contains(i))
		{
			QWidget* widget = ui.tabWidget->widget(j);
			Delete(widget);
			j--;
		}
	}

	ui.tabWidget->setCurrentIndex(0);

	if (obj->m_type == OT_MOVER || obj->m_type == OT_ITEM || obj->m_type == OT_CTRL)
	{
		CSpawnObject* spawn = (CSpawnObject*)obj;
		ui.spawn->setChecked(spawn->m_isRespawn);
		ui.spawnCount->setValue(spawn->m_count);
		ui.spawnAttackCount->setValue(spawn->m_attackCount);
		ui.spawnTime->setValue(spawn->m_time);
		ui.spawnDayMin->setValue(spawn->m_dayMin);
		ui.spawnDayMax->setValue(spawn->m_dayMax);
		ui.spawnHourMin->setValue(spawn->m_hourMin);
		ui.spawnHourMax->setValue(spawn->m_hourMax);
		ui.spawnItemMin->setValue(spawn->m_itemMin);
		ui.spawnItemMax->setValue(spawn->m_itemMax);

		connect(ui.spawn, SIGNAL(clicked(bool)), this, SLOT(SetSpawn(bool)));
		connect(ui.spawnCount, SIGNAL(valueChanged(int)), this, SLOT(SetSpawnCount(int)));

		if (obj->m_type == OT_MOVER)
		{
			CMover* mover = (CMover*)obj;
			CTextFile::FillComboBox(ui.moverAI, "AII_", mover->m_AIInterface);
			CTextFile::FillComboBox(ui.moverBelli, "BELLI_", mover->m_belligerence);
			Project->FillCharacterComboBox(ui.moverNPC, mover->m_characterKey);

			QVector<string> states;
			states.push_back("STATE_INIT");
			states.push_back("STATE_IDLE");
			states.push_back("STATE_WANDER");
			states.push_back("STATE_PURSUE");
			states.push_back("STATE_EVADE");
			states.push_back("STATE_RUNAWAY");
			states.push_back("STATE_RAGE");
			states.push_back("STATE_STAND");
			states.push_back("STATE_PATROL");
			states.push_back("STATE_RAGE_PATROL");
			for (int i = 0; i < states.size(); i++)
			{
				ui.moverAIState->addItem(states[i]);
				if (i + 1 == mover->m_aiState)
					ui.moverAIState->setCurrentIndex(i);
			}

			if (mover->m_world)
			{
				ui.moverPatrolIndex->addItem("-", -1);
				int i = 1;
				for (auto it = mover->m_world->m_paths.begin(); it != mover->m_world->m_paths.end(); it++)
				{
					ui.moverPatrolIndex->addItem("Path " + string::number(it.key()), it.key());
					if (it.key() == mover->m_patrolIndex)
						ui.moverPatrolIndex->setCurrentIndex(i);
					i++;
				}
				if (mover->m_patrolIndex == -1)
					ui.moverPatrolIndex->setCurrentIndex(0);
			}

			ui.moverPatrolCycle->setChecked(mover->m_patrolCycle != 0);

			connect(ui.moverPatrolIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(SetMoverPatrolIndex(int)));
			connect(ui.moverNPC, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(SetMoverCharacter(const QString&)));
		}
		else if (obj->m_type == OT_CTRL)
		{
			CCtrl* ctrl = (CCtrl*)obj;
			ui.ctrlKey->setText(ctrl->m_elem.strCtrlKey);
			ui.ctrlLinkKey->setText(ctrl->m_elem.strLinkCtrlKey);

			if (ctrl->m_elem.dwSet & UA_TELEPORT)
			{
				CTextFile::FillComboBox(ui.ctrlTeleWorld, "WI_", (int)ctrl->m_elem.dwTeleWorldId);
				ui.ctrlTeleWorldX->setValue((int)ctrl->m_elem.dwTeleX);
				ui.ctrlTeleWorldY->setValue((int)ctrl->m_elem.dwTeleY);
				ui.ctrlTeleWorldZ->setValue((int)ctrl->m_elem.dwTeleZ);
			}
			else
				CTextFile::FillComboBox(ui.ctrlTeleWorld, "WI_", 0);
			if (ctrl->m_elem.dwSet & UA_ITEM)
			{
				ui.ctrlCondItem->setChecked(true);
				ui.ctrlCondItemID->setValue((int)ctrl->m_elem.dwSetItem);
				ui.ctrlCondItemCount->setValue((int)ctrl->m_elem.dwSetItemCount);
			}
			if (ctrl->m_elem.dwSet & UA_LEVEL)
			{
				ui.ctrlCondLevel->setChecked(true);
				ui.ctrlCondMinLevel->setValue((int)ctrl->m_elem.dwSetLevel);
			}
			if (ctrl->m_elem.dwSet & UA_GENDER)
			{
				ui.ctrlCondSex->setChecked(true);
				CTextFile::FillComboBox(ui.ctrlCondSexType, "SEX_", (int)ctrl->m_elem.dwSetGender);
			}
			else
				CTextFile::FillComboBox(ui.ctrlCondSexType, "SEX_", SEX_SEXLESS);
			ui.ctrlCondEndu->setChecked(ctrl->m_elem.dwSetEndu > 0);
			ui.ctrlCondMinHP->setValue((int)ctrl->m_elem.dwSetEndu);
			if (ctrl->m_elem.dwSet & UA_QUEST)
			{
				ui.ctrlCondQuestBeg->setChecked(true);
				ui.ctrlCondQuest0ID->setValue((int)ctrl->m_elem.dwSetQuestNum);
				ui.ctrlCondQuest0Flag->setValue((int)ctrl->m_elem.dwSetFlagNum);
			}
			if (ctrl->m_elem.dwSet & UA_QUEST_END)
			{
				ui.ctrlCondQuestEnd->setChecked(true);
				ui.ctrlCondQuest1ID->setValue((int)ctrl->m_elem.dwSetQuestNum1);
				ui.ctrlCondQuest1Flag->setValue((int)ctrl->m_elem.dwSetFlagNum1);
				ui.ctrlCondQuest2ID->setValue((int)ctrl->m_elem.dwSetQuestNum2);
				ui.ctrlCondQuest2Flag->setValue((int)ctrl->m_elem.dwSetFlagNum2);
			}
			CTextFile::FillComboBox(ui.ctrlCondJobList, "JOB_", JOB_VAGRANT);
			if (ctrl->m_elem.dwSet & UA_CLASS)
			{
				QListWidgetItem* item;
				for (int i = 0; i < MAX_JOB; i++)
				{
					if (ctrl->m_elem.bSetJob[i])
					{
						item = new QListWidgetItem(CTextFile::GetDefineText(i, "JOB"), ui.ctrlCondJobs);
						item->setData(Qt::UserRole + 1, QVariant(i));
						ui.ctrlCondJobs->addItem(item);
					}
				}
			}

			ui.ctrlItemMin->setValue((int)ctrl->m_elem.dwMinItemNum);
			ui.ctrlItemMax->setValue((int)ctrl->m_elem.dwMaxiItemNum);
			ui.ctrlItem0ID->setValue((int)ctrl->m_elem.dwInsideItemKind[0]);
			ui.ctrlItem1ID->setValue((int)ctrl->m_elem.dwInsideItemKind[1]);
			ui.ctrlItem2ID->setValue((int)ctrl->m_elem.dwInsideItemKind[2]);
			ui.ctrlItem3ID->setValue((int)ctrl->m_elem.dwInsideItemKind[3]);
			ui.ctrlItem0Per->setValue((int)ctrl->m_elem.dwInsideItemPer[0]);
			ui.ctrlItem1Per->setValue((int)ctrl->m_elem.dwInsideItemPer[1]);
			ui.ctrlItem2Per->setValue((int)ctrl->m_elem.dwInsideItemPer[2]);
			ui.ctrlItem3Per->setValue((int)ctrl->m_elem.dwInsideItemPer[3]);

			ui.ctrlMon0ID->setValue((int)ctrl->m_elem.dwMonResKind[0]);
			ui.ctrlMon1ID->setValue((int)ctrl->m_elem.dwMonResKind[1]);
			ui.ctrlMon2ID->setValue((int)ctrl->m_elem.dwMonResKind[2]);
			ui.ctrlMon0Num->setValue((int)ctrl->m_elem.dwMonResNum[0]);
			ui.ctrlMon1Num->setValue((int)ctrl->m_elem.dwMonResNum[1]);
			ui.ctrlMon2Num->setValue((int)ctrl->m_elem.dwMonResNum[2]);
			ui.ctrlMon0Attack->setValue((int)ctrl->m_elem.dwMonActAttack[0]);
			ui.ctrlMon1Attack->setValue((int)ctrl->m_elem.dwMonActAttack[1]);
			ui.ctrlMon2Attack->setValue((int)ctrl->m_elem.dwMonActAttack[2]);

			ui.ctrlTrap0ID->setValue((int)ctrl->m_elem.dwTrapKind[0]);
			ui.ctrlTrap1ID->setValue((int)ctrl->m_elem.dwTrapKind[1]);
			ui.ctrlTrap2ID->setValue((int)ctrl->m_elem.dwTrapKind[2]);
			ui.ctrlTrap0Level->setValue((int)ctrl->m_elem.dwTrapLevel[0]);
			ui.ctrlTrap1Level->setValue((int)ctrl->m_elem.dwTrapLevel[1]);
			ui.ctrlTrap2Level->setValue((int)ctrl->m_elem.dwTrapLevel[2]);
			if (ctrl->m_elem.dwTrapOperType == TOT_NOENDU)
				ui.ctrlTrapNoEndu->setChecked(true);
			ui.ctrlTrapRandomPer->setValue((int)ctrl->m_elem.dwTrapRandomPer);
			ui.ctrlTrapDelay->setValue((int)ctrl->m_elem.dwTrapDelay);

			connect(ui.ctrlCondAddJob, SIGNAL(clicked()), this, SLOT(AddJob_()));
			connect(ui.ctrlCondRemoveJob, SIGNAL(clicked()), this, SLOT(RemoveJob()));
		}
	}
	else if (obj->m_type == OT_REGION)
	{
		CRegion* region = (CRegion*)obj;
		if (obj->m_modelID == RI_STRUCTURE)
		{
			ui.structureUnit->setChecked(region->m_directMusic);
			CTextFile::FillComboBox(ui.structureType, "SRT_", region->m_musicID);
		}
		else if (obj->m_modelID == RI_REVIVAL || obj->m_modelID == RI_PLACE)
		{
			ui.keyName->setText(region->m_key);
			ui.keyChao->setChecked(region->m_chaoKey);
			ui.keyTarget->setChecked(region->m_targetKey);
		}
		else if (obj->m_modelID == RI_ATTRIBUTE || obj->m_modelID == RI_TRIGGER)
		{
			QMap<uint, string> attributes;
			attributes[RA_WORLD] = "World Map";
			attributes[RA_DUNGEON] = "Dungeon Map";
			attributes[RA_NEWBIE] = "Newbie Map";
			attributes[RA_BEGIN] = "Begin Map";
			attributes[RA_SAFETY] = "Safety Region";
			attributes[RA_SHRINE] = "Shrine Region";
			attributes[RA_FIGHT] = "Fight Region";
			attributes[RA_SIGHT] = "Sight Region";
			attributes[RA_TOWN] = "Town Region";
			attributes[RA_DAYLIGHT] = "Daylight Region";
			attributes[RA_PENALTY_PK] = "Penalty PK Region";
			attributes[RA_PK] = "Free PK Region";
			attributes[RA_OX] = "OX Region";
			attributes[RA_DANGER] = "Danger Region";
			attributes[RA_NO_CHAT] = "No Chat Region";
			attributes[RA_NO_ATTACK] = "No Attack Region";
			attributes[RA_NO_DAMAGE] = "No Damage Region";
			attributes[RA_NO_SKILL] = "No Skill Region";
			attributes[RA_NO_ITEM] = "No Item Region";
			attributes[RA_NO_TELEPORT] = "No Teleport Region";
			attributes[RA_COLLECTING] = "Collecting Region";

			QListWidgetItem* item;
			for (auto it = attributes.begin(); it != attributes.end(); it++)
			{
				item = new QListWidgetItem(it.value(), ui.regionAttributes);
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
				item->setCheckState((region->m_attributes & it.key()) ? Qt::Checked : Qt::Unchecked);
				item->setData(Qt::UserRole + 1, QVariant(it.key()));
				ui.regionAttributes->addItem(item);
			}

			ui.regionBGM->setChecked(region->m_directMusic);
			CTextFile::FillComboBox(ui.regionBGMID, "BGM_", region->m_musicID);
			ui.regionScript->setText(region->m_script);
			ui.regionSound->setText(region->m_sound);
			ui.regionTitle->setText(region->m_title.replace("\\n", "\n"));
			ui.regionDesc->setText(region->m_desc.replace("\\n", "\n"));
			CTextFile::FillComboBox(ui.regionTeleWorld, "WI_", region->m_teleportWorldID);
			ui.regionTeleWorldX->setValue((double)region->m_teleportWorldPos.x);
			ui.regionTeleWorldY->setValue((double)region->m_teleportWorldPos.y);
			ui.regionTeleWorldZ->setValue((double)region->m_teleportWorldPos.z);

			if (region->m_itemID != -1 && region->m_itemCount != -1)
			{
				ui.regionCondItem->setChecked(true);
				ui.regionCondItemID->setValue(region->m_itemID);
				ui.regionCondItemCount->setValue(region->m_itemCount);
			}
			if (region->m_minLevel != -1 && region->m_maxLevel != -1)
			{
				ui.regionCondLevel->setChecked(true);
				ui.regionCondMinLevel->setValue(region->m_minLevel);
				ui.regionCondMaxLevel->setValue(region->m_maxLevel);
			}
			if (region->m_jobID != -1)
			{
				ui.regionCondJob->setChecked(true);
				ui.regionCondJobID->setValue(region->m_jobID);
			}
			if (region->m_gender != -1)
			{
				ui.regionCondSex->setChecked(true);
				ui.regionCondSexType->setCurrentIndex(region->m_gender);
			}
			if (region->m_questID != -1)
			{
				ui.regionCondQuest->setChecked(true);
				ui.regionCondQuestID->setValue(region->m_questID);
				ui.regionCondQuestFlag->setValue(region->m_questFlag);
			}
			if (region->m_checkGuild)
				ui.regionCondGuild->setChecked(true);
			if (region->m_checkParty)
				ui.regionCondParty->setChecked(true);
		}
	}
	else if (obj->m_type == OT_PATH)
	{
		ui.pathMotion->addItem("MTI_WALK", MTI_WALK);
		ui.pathMotion->addItem("MTI_RUN", MTI_RUN);

		CPath* path = (CPath*)obj;
		if (path->m_motion == MTI_WALK)
			ui.pathMotion->setCurrentIndex(0);
		else if (path->m_motion == MTI_RUN)
			ui.pathMotion->setCurrentIndex(1);

		ui.pathDelay->setValue(path->m_delay);
	}

	connect(this, SIGNAL(finished(int)), this, SLOT(SaveProperties(int)));
	adjustSize();

	m_obj = obj;
}

void CObjectPropertiesDialog::SaveProperties(int result)
{
	if (!m_obj)
		return;

	if (m_obj->m_type == OT_MOVER || m_obj->m_type == OT_ITEM || m_obj->m_type == OT_CTRL)
	{
		CSpawnObject* spawn = (CSpawnObject*)m_obj;
		spawn->m_attackCount = ui.spawnAttackCount->value();
		spawn->m_time = ui.spawnTime->value();
		spawn->m_dayMin = ui.spawnDayMin->value();
		spawn->m_dayMax = ui.spawnDayMax->value();
		spawn->m_hourMin = ui.spawnHourMin->value();
		spawn->m_hourMax = ui.spawnHourMax->value();
		spawn->m_itemMin = ui.spawnItemMin->value();
		spawn->m_itemMax = ui.spawnItemMax->value();

		if (m_obj->m_type == OT_MOVER)
		{
			CMover* mover = (CMover*)m_obj;
			mover->m_AIInterface = (uint)ui.moverAI->itemData(ui.moverAI->currentIndex()).toInt();
			mover->m_belligerence = (uint)ui.moverBelli->itemData(ui.moverBelli->currentIndex()).toInt();
			mover->m_aiState = ui.moverAIState->currentIndex() + 1;
			mover->m_patrolCycle = ui.moverPatrolCycle->isChecked() ? 1 : 0;
		}
		else if (m_obj->m_type == OT_CTRL)
		{
			CCtrl* ctrl = (CCtrl*)m_obj;
			ctrl->m_elem.dwSet = 0;

			QByteArray temp = ui.ctrlKey->text().toLocal8Bit();
			memcpy(ctrl->m_elem.strCtrlKey, temp.constData(), temp.size());
			ctrl->m_elem.strCtrlKey[temp.size()] = '\0';
			temp = ui.ctrlLinkKey->text().toLocal8Bit();
			memcpy(ctrl->m_elem.strLinkCtrlKey, temp.constData(), temp.size());
			ctrl->m_elem.strLinkCtrlKey[temp.size()] = '\0';

			ctrl->m_elem.dwTeleWorldId = (uint)ui.ctrlTeleWorld->itemData(ui.ctrlTeleWorld->currentIndex()).toInt();
			ctrl->m_elem.dwTeleX = (uint)ui.ctrlTeleWorldX->value();
			ctrl->m_elem.dwTeleY = (uint)ui.ctrlTeleWorldY->value();
			ctrl->m_elem.dwTeleZ = (uint)ui.ctrlTeleWorldZ->value();
			if (ctrl->m_elem.dwTeleWorldId != WI_WORLD_NONE)
				ctrl->m_elem.dwSet |= UA_TELEPORT;

			if (ui.ctrlCondItem->isChecked())
			{
				ctrl->m_elem.dwSetItem = (uint)ui.ctrlCondItemID->value();
				ctrl->m_elem.dwSetItemCount = (uint)ui.ctrlCondItemCount->value();
				ctrl->m_elem.dwSet |= UA_ITEM;
			}
			else
			{
				ctrl->m_elem.dwSetItem = 0;
				ctrl->m_elem.dwSetItemCount = 0;
			}
			if (ui.ctrlCondLevel->isChecked())
			{
				ctrl->m_elem.dwSetLevel = (uint)ui.ctrlCondMinLevel->value();
				ctrl->m_elem.dwSet |= UA_LEVEL;
			}
			else
				ctrl->m_elem.dwSetLevel = 0;
			if (ui.ctrlCondSex->isChecked())
			{
				ctrl->m_elem.dwSetGender = (uint)ui.ctrlCondSexType->itemData(ui.ctrlCondSexType->currentIndex()).toInt();
				ctrl->m_elem.dwSet |= UA_GENDER;
			}
			else
				ctrl->m_elem.dwSetGender = 0;
			if (ui.ctrlCondEndu->isChecked())
				ctrl->m_elem.dwSetEndu = (uint)ui.ctrlCondMinHP->value();
			else
				ctrl->m_elem.dwSetEndu = 0;
			if (ui.ctrlCondQuestBeg->isChecked())
			{
				ctrl->m_elem.dwSetQuestNum = (uint)ui.ctrlCondQuest0ID->value();
				ctrl->m_elem.dwSetFlagNum = (uint)ui.ctrlCondQuest0Flag->value();
				ctrl->m_elem.dwSet |= UA_QUEST;
			}
			else
			{
				ctrl->m_elem.dwSetQuestNum = 0;
				ctrl->m_elem.dwSetFlagNum = 0;
			}
			if (ui.ctrlCondQuestEnd->isChecked())
			{
				ctrl->m_elem.dwSetQuestNum1 = (uint)ui.ctrlCondQuest1ID->value();
				ctrl->m_elem.dwSetFlagNum1 = (uint)ui.ctrlCondQuest1Flag->value();
				ctrl->m_elem.dwSetQuestNum2 = (uint)ui.ctrlCondQuest2ID->value();
				ctrl->m_elem.dwSetFlagNum2 = (uint)ui.ctrlCondQuest2Flag->value();
				ctrl->m_elem.dwSet |= UA_QUEST_END;
			}
			else
			{
				ctrl->m_elem.dwSetQuestNum1 = 0;
				ctrl->m_elem.dwSetFlagNum1 = 0;
				ctrl->m_elem.dwSetQuestNum2 = 0;
				ctrl->m_elem.dwSetFlagNum2 = 0;
			}
			memset(ctrl->m_elem.bSetJob, 0, sizeof(ctrl->m_elem.bSetJob));
			if (ui.ctrlCondJobs->count() > 0)
			{
				ctrl->m_elem.dwSet |= UA_CLASS;
				for (int i = 0; i < ui.ctrlCondJobs->count(); i++)
					ctrl->m_elem.bSetJob[ui.ctrlCondJobs->item(i)->data(Qt::UserRole + 1).toInt()] = 1;
			}

			ctrl->m_elem.dwMinItemNum = (uint)ui.ctrlItemMin->value();
			ctrl->m_elem.dwMaxiItemNum = (uint)ui.ctrlItemMax->value();
			ctrl->m_elem.dwInsideItemKind[0] = (uint)ui.ctrlItem0ID->value();
			ctrl->m_elem.dwInsideItemKind[1] = (uint)ui.ctrlItem1ID->value();
			ctrl->m_elem.dwInsideItemKind[2] = (uint)ui.ctrlItem2ID->value();
			ctrl->m_elem.dwInsideItemKind[3] = (uint)ui.ctrlItem3ID->value();
			ctrl->m_elem.dwInsideItemPer[0] = (uint)ui.ctrlItem0Per->value();
			ctrl->m_elem.dwInsideItemPer[1] = (uint)ui.ctrlItem1Per->value();
			ctrl->m_elem.dwInsideItemPer[2] = (uint)ui.ctrlItem2Per->value();
			ctrl->m_elem.dwInsideItemPer[3] = (uint)ui.ctrlItem3Per->value();

			ctrl->m_elem.dwMonResKind[0] = (uint)ui.ctrlMon0ID->value();
			ctrl->m_elem.dwMonResKind[1] = (uint)ui.ctrlMon1ID->value();
			ctrl->m_elem.dwMonResKind[2] = (uint)ui.ctrlMon2ID->value();
			ctrl->m_elem.dwMonResNum[0] = (uint)ui.ctrlMon0Num->value();
			ctrl->m_elem.dwMonResNum[1] = (uint)ui.ctrlMon1Num->value();
			ctrl->m_elem.dwMonResNum[2] = (uint)ui.ctrlMon2Num->value();
			ctrl->m_elem.dwMonActAttack[0] = (uint)ui.ctrlMon0Attack->value();
			ctrl->m_elem.dwMonActAttack[1] = (uint)ui.ctrlMon1Attack->value();
			ctrl->m_elem.dwMonActAttack[2] = (uint)ui.ctrlMon2Attack->value();

			ctrl->m_elem.dwTrapKind[0] = (uint)ui.ctrlTrap0ID->value();
			ctrl->m_elem.dwTrapKind[1] = (uint)ui.ctrlTrap1ID->value();
			ctrl->m_elem.dwTrapKind[2] = (uint)ui.ctrlTrap2ID->value();
			ctrl->m_elem.dwTrapLevel[0] = (uint)ui.ctrlTrap0Level->value();
			ctrl->m_elem.dwTrapLevel[1] = (uint)ui.ctrlTrap1Level->value();
			ctrl->m_elem.dwTrapLevel[2] = (uint)ui.ctrlTrap2Level->value();
			if (ui.ctrlTrapNoEndu->isChecked())
				ctrl->m_elem.dwTrapOperType = TOT_NOENDU;
			else
				ctrl->m_elem.dwTrapOperType = TOT_RANDOM;
			ctrl->m_elem.dwTrapRandomPer = (uint)ui.ctrlTrapRandomPer->value();
			ctrl->m_elem.dwTrapDelay = (uint)ui.ctrlTrapDelay->value();
		}
	}
	else if (m_obj->m_type == OT_REGION)
	{
		CRegion* region = (CRegion*)m_obj;
		if (m_obj->m_modelID == RI_STRUCTURE)
		{
			region->m_directMusic = ui.structureUnit->isChecked();
			region->m_musicID = ui.structureType->itemData(ui.structureType->currentIndex()).toInt();
		}
		else if (m_obj->m_modelID == RI_REVIVAL || m_obj->m_modelID == RI_PLACE)
		{
			region->m_key = ui.keyName->text();
			region->m_chaoKey = ui.keyChao->isChecked();
			region->m_targetKey = ui.keyTarget->isChecked();
		}
		else if (m_obj->m_modelID == RI_ATTRIBUTE || m_obj->m_modelID == RI_TRIGGER)
		{
			QListWidgetItem* item;
			region->m_attributes = 0;
			for (int i = 0; i < ui.regionAttributes->count(); i++)
			{
				item = ui.regionAttributes->item(i);
				if (item->checkState() == Qt::Checked)
					region->m_attributes |= item->data(Qt::UserRole + 1).toUInt();
			}

			region->m_directMusic = ui.regionBGM->isChecked();
			region->m_musicID = ui.regionBGMID->itemData(ui.regionBGMID->currentIndex()).toInt();
			region->m_script = ui.regionScript->text();
			region->m_sound = ui.regionSound->text();
			region->m_title = ui.regionTitle->toPlainText().replace("\n", "\\n");
			region->m_desc = ui.regionDesc->toPlainText().replace("\n", "\\n");
			region->m_teleportWorldID = ui.regionTeleWorld->itemData(ui.regionTeleWorld->currentIndex()).toInt();
			region->m_teleportWorldPos.x = (float)ui.regionTeleWorldX->value();
			region->m_teleportWorldPos.y = (float)ui.regionTeleWorldY->value();
			region->m_teleportWorldPos.z = (float)ui.regionTeleWorldZ->value();

			if (ui.regionCondItem->isChecked())
			{
				region->m_itemID = ui.regionCondItemID->value();
				region->m_itemCount = ui.regionCondItemCount->value();
			}
			else
			{
				region->m_itemID = -1;
				region->m_itemCount = -1;
			}
			if (ui.regionCondLevel->isChecked())
			{
				region->m_minLevel = ui.regionCondMinLevel->value();
				region->m_maxLevel = ui.regionCondMaxLevel->value();
			}
			else
			{
				region->m_minLevel = -1;
				region->m_maxLevel = -1;
			}
			if (ui.regionCondJob->isChecked())
				region->m_jobID = ui.regionCondJobID->value();
			else
				region->m_jobID = -1;
			if (ui.regionCondSex->isChecked())
				region->m_gender = ui.regionCondSexType->itemData(ui.regionCondSexType->currentIndex()).toInt();
			else
				region->m_gender = -1;
			if (ui.regionCondQuest->isChecked())
			{
				region->m_questID = ui.regionCondQuestID->value();
				region->m_questFlag = ui.regionCondQuestFlag->value();
			}
			else
			{
				region->m_questID = -1;
				region->m_questFlag = -1;
			}
			region->m_checkGuild = ui.regionCondGuild->isChecked();
			region->m_checkParty = ui.regionCondParty->isChecked();
		}
	}
	else if (m_obj->m_type == OT_PATH)
	{
		CPath* path = (CPath*)m_obj;
		path->m_motion = ui.pathMotion->itemData(ui.pathMotion->currentIndex()).toInt();
		path->m_delay = ui.pathDelay->value();
	}
}

void CObjectPropertiesDialog::SetSpawn(bool spawn)
{
	if (!m_obj)
		return;

	((CSpawnObject*)m_obj)->m_isRespawn = spawn;
	if (g_global3D.spawnAllMovers && m_obj->m_world)
		m_obj->m_world->SpawnObject(m_obj, spawn);

	MainFrame->UpdateWorldEditor();
}

void CObjectPropertiesDialog::SetSpawnCount(int count)
{
	if (!m_obj)
		return;

	((CSpawnObject*)m_obj)->m_count = count;
	if (g_global3D.spawnAllMovers && m_obj->m_world)
	{
		m_obj->m_world->SpawnObject(m_obj, false);
		m_obj->m_world->SpawnObject(m_obj, true);
	}

	MainFrame->UpdateWorldEditor();
}

void CObjectPropertiesDialog::SetMoverCharacter(const QString& key)
{
	if (!m_obj)
		return;

	CMover* mover = (CMover*)m_obj;
	if (key == "-")
		mover->m_characterKey = "";
	else
		mover->m_characterKey = key;

	if (mover->m_modelProp && mover->m_modelProp->modelType != MODELTYPE_MESH)
		Delete(mover->m_model);

	mover->m_character = null;
	mover->m_model = null;
	mover->Init();

	MainFrame->UpdateWorldEditor();
}

void CObjectPropertiesDialog::AddJob_()
{
	if (!m_obj)
		return;

	const int jobID = ui.ctrlCondJobList->itemData(ui.ctrlCondJobList->currentIndex()).toInt();

	QListWidgetItem* item;
	for (int i = 0; i < ui.ctrlCondJobs->count(); i++)
	{
		item = ui.ctrlCondJobs->item(i);
		if (item->data(Qt::UserRole + 1).toInt() == jobID)
			return;
	}

	item = new QListWidgetItem(CTextFile::GetDefineText(jobID, "JOB"), ui.ctrlCondJobs);
	item->setData(Qt::UserRole + 1, QVariant(jobID));
	ui.ctrlCondJobs->addItem(item);
}

void CObjectPropertiesDialog::RemoveJob()
{
	if (!m_obj || ui.ctrlCondJobs->count() < 0)
		return;

	const int row = ui.ctrlCondJobs->currentRow();
	if (row != -1)
	{
		QListWidgetItem* item = ui.ctrlCondJobs->takeItem(row);
		Delete(item);
	}
}

void CObjectPropertiesDialog::SetMoverPatrolIndex(int index)
{
	if (!m_obj)
		return;

	((CMover*)m_obj)->m_patrolIndex = ui.moverPatrolIndex->itemData(ui.moverPatrolIndex->currentIndex()).toInt();
	MainFrame->UpdateWorldEditor();
}