///////////
// This file is a part of the ATools project
// Some parts of code are the property of Microsoft, Qt or Aeonsoft
// The rest is released without license and without any warranty
///////////

#ifndef TERRAINEDITION_H
#define TERRAINEDITION_H

#include "EditCommand.h"
#include <Landscape.h>

class CEditTerrainHeightCommand : public CEditCommand
{
public:
	CEditTerrainHeightCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_heights.empty();
	}

	void Edit(const D3DXVECTOR3& pos, const QPoint& mousePos, float baseHeight);

private:
	struct HeightEntry {
		int offset;
		float original;
		float height;
	};
	std::vector<HeightEntry> m_heights;
};

class CEditWaterCommand : public CEditCommand
{
public:
	CEditWaterCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_heights.empty();
	}

	void Edit(const D3DXVECTOR3& pos);
	void Optimize();
	void FillAllMap();

private:
	struct HeightEntry {
		int offset;
		WaterHeight original;
		WaterHeight height;
	};
	std::vector<HeightEntry> m_heights;
};

class CEditTerrainColorCommand : public CEditCommand
{
public:
	CEditTerrainColorCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_colors.empty();
	}

	void Edit(const D3DXVECTOR3& pos);

private:
	struct ColorEntry {
		int offset;
		byte original[3];
		byte color[3];
	};
	std::vector<ColorEntry> m_colors;
};

class CEditTerrainTextureCommand : public CEditCommand
{
public:
	CEditTerrainTextureCommand(CWorld* world)
		: CEditCommand(world) { }

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_layers.empty();
	}

	void Edit(const D3DXVECTOR3& pos);

private:
	struct AlphaEntry
	{
		int offset;
		byte original;
		byte alpha;
	};
	struct LayerEntry
	{
		int texID;
		std::vector<AlphaEntry> alphas;
	};
	std::vector<LayerEntry> m_layers;

	void _setAlpha(int texID, int offset, byte alpha);
};

class CDeleteTerrainLayerCommand : public CEditCommand
{
public:
	CDeleteTerrainLayerCommand(CWorld* world, CLandscape* land, int layerID);

	virtual void Apply(bool undo = false);
	virtual bool IsEmpty() const {
		return m_position == -1;
	}

private:
	byte m_layerAlpha[MAP_SIZE * MAP_SIZE];
	bool m_layerPatchEnable[NUM_PATCHES_PER_SIDE * NUM_PATCHES_PER_SIDE];
	int m_layerID;
	CLandscape* m_land;
	int m_position;
};

#endif // TERRAINEDITION_H