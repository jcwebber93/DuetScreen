#include "HeightmapView.h"
#include "Debug.h"
#include "UI/Core/Navigation.h"
#include "i18n/i18n.h"
#include "utils/DisplayHelper.h"

#include "ObjectModel/Heightmap.h"
#include <cmath>

namespace UI
{
	/* Portrait: no width for a side column, so the heatmap takes the top and every control stacks beneath it. */
	static constexpr int32_t s_portraitColDsc[2] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
	static constexpr int32_t s_portraitRowDsc[6] = {
		LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

	class HeightmapItem : public ListItem
	{
	  public:
		HeightmapItem(size_t index, LvObj& parent, HeightmapView& view)
			: ListItem(index, parent)
			, m_view(view)
			, m_label("label", getRoot())
			, m_load("load", getRoot(), "", layout_t(0, 0, LV_SIZE_CONTENT, LV_SIZE_CONTENT))
		{
			ZoneScoped;
			UI_LOCK();

			setFlexFlow(LV_FLEX_FLOW_ROW);
			setFlexAlign(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
			setSize(LV_PCT(100), LV_SIZE_CONTENT);
			m_label.setHeight(LV_SIZE_CONTENT);
			m_label.setFlexGrow(1);

			setFlag(LV_OBJ_FLAG_CLICKABLE, true);
			addEventCallback(
				[](lv_event_t* event)
				{
					ZoneScopedN("HeightmapItem::click");
					UI_LOCK();
					auto item = static_cast<HeightmapItem*>(lv_event_get_user_data(event));
					if (item == nullptr)
					{
						LOG_WARN("Heightmap item is null");
						return;
					}
					item->m_view.getPresenter()->setActiveHeightmap(item->getIndex());
				},
				LV_EVENT_CLICKED,
				this);

			m_load.addClickedCallback(
				[](lv_event_t* event)
				{
					ZoneScopedN("HeightmapItem::load");
					UI_LOCK();
					auto item = static_cast<HeightmapItem*>(lv_event_get_user_data(event));
					if (item == nullptr)
					{
						LOG_WARN("Heightmap item is null");
						return;
					}
					item->m_view.getPresenter()->toggleHeightmap(item->getIndex());
					item->m_view.getPresenter()->setActiveHeightmap(item->getIndex());
				},
				this);
			// m_load.setCheckable(true);

			m_load.setSize(70, LV_SIZE_CONTENT);
			m_load.setMinWidth(LV_SIZE_CONTENT);
			addStyle(Themes::getLvglStyles().bg_color_primary, LV_STATE_CHECKED);
			m_load.addStyle(Themes::getLvglStyles().actionBtn, 0);
		}

		void setLabel(std::string_view label)
		{
			ZoneScoped;
			UI_LOCK();
			m_label.setText(label);
		}

		void setSelected(bool selected)
		{
			ZoneScoped;
			UI_LOCK();
			setState(LV_STATE_CHECKED, selected);
			m_load.setText(selected ? _("heightmap.unload") : _("heightmap.load"));
			m_load.setChecked(selected);
		}

	  private:
		HeightmapView& m_view;

		LvLabel m_label;
		Button m_load;
	};

	HeightmapRenderMode::HeightmapRenderMode(LvObj& parent, HeightmapPresenter& presenter)
		: LvContainer("heightmap_render_mode", parent)
		, m_presenter(presenter)
		, m_title("label", getRoot())
		, m_btns("btns", getRoot())
		, m_fixed("heightmap_fixed", m_btns, _("heightmap.fixed"), layout_t(0, 0, LV_SIZE_CONTENT, LV_SIZE_CONTENT))
		, m_auto("heightmap_auto", m_btns, _("heightmap.auto"), layout_t(0, 0, LV_SIZE_CONTENT, LV_SIZE_CONTENT))
	{
		ZoneScoped;
		UI_LOCK();
		setFlexFlow(LV_FLEX_FLOW_COLUMN);
		m_title.setText(_("heightmap.render_mode"));

		m_btns.setFlexFlow(LV_FLEX_FLOW_ROW);
		m_btns.setFlexAlign(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

		setSize(LV_PCT(100), LV_SIZE_CONTENT);
		m_btns.setSize(LV_PCT(100), LV_SIZE_CONTENT);
		m_fixed.setHeight(LV_SIZE_CONTENT);
		m_auto.setHeight(LV_SIZE_CONTENT);

		m_fixed.setFlexGrow(1);
		m_auto.setFlexGrow(1);

		setRenderMode(m_presenter.getRenderMode());

		m_fixed.addClickedCallback(
			[](lv_event_t* event)
			{
				ZoneScopedN("HeightmapRenderMode::fixed");
				UI_LOCK();
				auto mode = static_cast<HeightmapRenderMode*>(lv_event_get_user_data(event));
				if (mode == nullptr)
				{
					LOG_WARN("Heightmap render mode is null");
					return;
				}
				mode->m_presenter.setRenderMode(HeightmapPresenter::HeightmapRenderMode::Fixed);
				mode->setRenderMode(HeightmapPresenter::HeightmapRenderMode::Fixed);
			},
			this);

		m_auto.addClickedCallback(
			[](lv_event_t* event)
			{
				ZoneScopedN("HeightmapRenderMode::auto");
				UI_LOCK();
				auto mode = static_cast<HeightmapRenderMode*>(lv_event_get_user_data(event));
				if (mode == nullptr)
				{
					LOG_WARN("Heightmap render mode is null");
					return;
				}
				mode->m_presenter.setRenderMode(HeightmapPresenter::HeightmapRenderMode::Auto);
				mode->setRenderMode(HeightmapPresenter::HeightmapRenderMode::Auto);
			},
			this);
	}

	void HeightmapRenderMode::setRenderMode(HeightmapPresenter::HeightmapRenderMode mode)
	{
		ZoneScoped;
		UI_LOCK();
		switch (mode)
		{
		case HeightmapPresenter::HeightmapRenderMode::Fixed:
			m_fixed.setChecked(true);
			m_auto.setChecked(false);
			break;
		case HeightmapPresenter::HeightmapRenderMode::Auto:
			m_fixed.setChecked(false);
			m_auto.setChecked(true);
			break;
		default:
			break;
		}
	}

	HeightmapStatistics::HeightmapStatistics(const std::string& name, LvObj& parent)
		: LvContainer(name, parent)
		, m_numPoints("num_points", getRoot())
		, m_area("area", getRoot())
		, m_minError("min_error", getRoot())
		, m_maxError("max_error", getRoot())
		, m_meanError("mean_error", getRoot())
		, m_stdDev("std_dev", getRoot())
	{
		ZoneScoped;
		setSize(LV_PCT(100), LV_SIZE_CONTENT);
		setFlexFlow(LV_FLEX_FLOW_ROW_WRAP);
		setFlexAlign(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
		iterateChildren([](size_t /* index */, LvObj& child) { child.setSize(LV_SIZE_CONTENT, LV_SIZE_CONTENT); });
	}

	void HeightmapStatistics::setStatistics(
		size_t numPoints, double area, double minError, double maxError, double meanError, double stdDev)
	{
		ZoneScoped;
		UI_LOCK();
		m_numPoints.setText(_("heightmap.num_points", numPoints));
		m_area.setText(_("heightmap.area", area));
		m_minError.setText(_("heightmap.min_error", minError));
		m_maxError.setText(_("heightmap.max_error", maxError));
		m_meanError.setText(_("heightmap.mean_error", meanError));
		m_stdDev.setText(_("heightmap.std_dev", stdDev));
	}

	HeightmapView::HeightmapView(const std::string& name, LvObj& parent)
		: View(name, parent, layout_t(0, 0, 100, 100))
		, m_heightmapList("heightmap_list", getRoot())
	{
		ZoneScoped;
		UI_LOCK();

		addStyle(Themes::getLvglStyles().bg_dark);
		m_heightmap.addStyle(Themes::getLvglStyles().card);
		m_heightmapList.addStyle(Themes::getLvglStyles().card);
		m_statistics.addStyle(Themes::getLvglStyles().card);
		m_renderMode.addStyle(Themes::getLvglStyles().card);
		m_trueBedLevel.addStyle(Themes::getLvglStyles().actionBtn, 0);
		m_meshBedLevel.addStyle(Themes::getLvglStyles().actionBtn, 0);

		if (Display::isPortrait())
		{
			setGridDsc(s_portraitColDsc, s_portraitRowDsc);
			setGridCell(m_heightmap, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
			setGridCell(m_btnCont, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
			setGridCell(m_heightmapList, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 2, 1);
			setGridCell(m_renderMode, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 3, 1);
			setGridCell(m_statistics, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 4, 1);
		}
		else
		{
			setGridDsc(m_layoutColDsc, m_layoutRowDsc);
			setGridCell(m_heightmap, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 3);
			setGridCell(m_btnCont, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);
			setGridCell(m_heightmapList, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
			setGridCell(m_renderMode, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 2, 1);
			setGridCell(m_statistics, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 3, 1);
		}

		m_heightmap.setResolution(200, 200);

		m_btnCont.setSize(LV_PCT(100), LV_SIZE_CONTENT);
		m_btnCont.setFlexFlow(LV_FLEX_FLOW_ROW);
		m_btnCont.setFlexAlign(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

		/* Need to set the width to override default LV_SIZE_CONTENT which prevents text wrapping. Value doesn't matter
		 * since it uses flex grow */
		m_trueBedLevel.setSize(0, LV_PCT(100));
		m_meshBedLevel.setSize(0, LV_PCT(100));
		m_trueBedLevel.setMinHeight(LV_SIZE_CONTENT);
		m_meshBedLevel.setMinHeight(LV_SIZE_CONTENT);
		m_trueBedLevel.setFlexGrow(1);
		m_meshBedLevel.setFlexGrow(1);
		m_trueBedLevel.setText(_("heightmap.true_bed_level"));
		m_meshBedLevel.setText(_("heightmap.mesh_bed_level"));
		m_trueBedLevel.addClickedCallback(onTrueBedLevelEvent, this);
		m_meshBedLevel.addClickedCallback(onMeshBedLevelEvent, this);

		// List
		m_heightmapList.setTitle(_("heightmap.list_header"));
		if (Display::isPortrait())
		{
			/* Hug the single visible entry instead of filling the cell */
			m_heightmapList.setSize(LV_PCT(100), LV_SIZE_CONTENT);
		}
		else
		{
			m_heightmapList.setListGrow(1);
		}

		clear();
	}

	HeightmapView::~HeightmapView() = default;

	size_t HeightmapView::getHeightmapCount() const
	{
		ZoneScoped;
		UI_LOCK();
		return m_heightmapList.getItemCount();
	}

	void HeightmapView::setHeightmapCount(const size_t count)
	{
		ZoneScoped;
		UI_LOCK();
		m_heightmapList.setItemCount(count, *this);

		if (Display::isPortrait() && count > 0)
		{
			/* Vertical space belongs to the heatmap, so only one saved map is shown and the rest are scrolled to.
			 * The height is taken from a real item so it tracks the font and padding of the active theme. */
			if (auto item = m_heightmapList.getItem(0); item != nullptr)
			{
				item->updateLayout();
				const int32_t itemHeight = item->getHeight();
				if (itemHeight > 0)
				{
					m_heightmapList.getListContainer().setMaxHeight(itemHeight);
				}
			}
		}
	}

	void HeightmapView::setHeightmapName(const size_t index, const std::string& name)
	{
		ZoneScoped;
		UI_LOCK();

		auto item = m_heightmapList.getItem(index);
		if (!item)
		{
			LOG_WARN("Can't set heightmap {:d} name to {:s}, index invalid", index, name);
			return;
		}

		item->setLabel(name);
	}

	void HeightmapView::setSelectedHeightmap(const size_t index)
	{
		ZoneScoped;
		UI_LOCK();
		for (auto& item : m_heightmapList)
		{
			if (item == nullptr)
			{
				LOG_WARN("Heightmap item is null");
				continue;
			}
			item->setSelected(item->getIndex() == index);
		}
	}

	void HeightmapView::setShownHeightmapName(std::string_view name)
	{
		ZoneScoped;
		UI_LOCK();
		m_heightmap.setTitle(_("heightmap.title", name));
	}

	void HeightmapView::addMeasurementPoint(float x, float y)
	{
		ZoneScoped;
		UI_LOCK();
		size_t px, py;
		if (!m_heightmap.posToPx(x, y, px, py))
		{
			LOG_WARN("Measurement ({:g}, {:g}) not within axis bounds", x, y);
			return;
		}
		uint32_t width, height;
		m_heightmap.getResolution(width, height);
		m_heightmap.getCanvas().drawCirclePx(
			{(int32_t)px, (int32_t)(height - py)}, 1, lv_palette_main(LV_PALETTE_GREY), LV_OPA_20);
	}

	void HeightmapView::clear()
	{
		ZoneScoped;
		UI_LOCK();
		m_heightmap.clear();

		setShownHeightmapName("");
		drawGrid();
		setStatistics(0, 0.0, 0.0, 0.0, 0.0, 0.0);

		m_heightmapList.clear();
	}

	void HeightmapView::setStatistics(
		size_t numPoints, double area, double minError, double maxError, double meanError, double stdDev)
	{
		ZoneScoped;
		UI_LOCK();
		m_statistics.setStatistics(numPoints, area, minError, maxError, meanError, stdDev);
	}

	void HeightmapView::onTrueBedLevelEvent(lv_event_t* e)
	{
		ZoneScoped;
		UI_LOCK();
		auto view = static_cast<HeightmapView*>(lv_event_get_user_data(e));
		view->m_presenter->trueBedLevel();
	}

	void HeightmapView::onMeshBedLevelEvent(lv_event_t* e)
	{
		ZoneScoped;
		UI_LOCK();
		auto view = static_cast<HeightmapView*>(lv_event_get_user_data(e));
		view->m_presenter->meshBedLevel();
	}

	void HeightmapView::onShow() {}
} // namespace UI
