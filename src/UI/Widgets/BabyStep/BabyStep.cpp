/*
 * BabyStep.cpp
 *
 *  Created on: 2025-09-25
 *      Author: Andy Everitt
 */

#include "BabyStep.h"
#include "Debug.h"
#include "UI/Styles/Styles.h"
#include "i18n/i18n.h"
#include "utils/StorageHelper.h"
#include "utils/UnitSystem.h"

namespace UI
{
	BabyStep::BabyStep(const std::string& name, LvObj& parent)
		: View(name, parent)
	{
		ZoneScoped;
		UI_LOCK();

		setFlexFlow(LV_FLEX_FLOW_COLUMN);
		setFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

		m_header.setText(_("babystep.header"));

		m_buttonPanel.setFlexGrow(1);
		m_buttonPanel.setWidth(LV_PCT(100));
		// m_buttonPanel.setMinWidth(LV_SIZE_CONTENT);

		m_buttonPanel.setIncrementIcon("babystep_increment.png");
		m_buttonPanel.setDecrementIcon("babystep_decrement.png");
		m_buttonPanel.setValueLabelFmt("{:g} mm");
		m_buttonPanel.setValueChangeCallback([this](float change) { m_presenter->babystep(change); });
		m_buttonPanel.setResetCallback([this]() { m_presenter->resetBabystep(); });
		m_buttonPanel.setUpdatedValuesCallback([this](const std::vector<float>& values)
											   { StorageHelper::setData(ID_BABYSTEP_AMOUNT, values); });
		m_buttonPanel.setMinValue(0.001f);
		m_buttonPanel.setMaxValue(1.0f);

		m_buttonPanel.getResetButton().addStyle(Themes::getLvglStyles().actionBtn);
		m_buttonPanel.getIncrementButton().addStyle(Themes::getLvglStyles().actionBtn);
		m_buttonPanel.getDecrementButton().addStyle(Themes::getLvglStyles().actionBtn);
	}

	void BabyStep::setHorizontal(bool horizontal)
	{
		ZoneScoped;
		UI_LOCK();
		setFlexFlow(horizontal ? LV_FLEX_FLOW_ROW : LV_FLEX_FLOW_COLUMN);
		setFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		m_buttonPanel.setHorizontal(horizontal);
		if (horizontal)
		{
			/* The header sits to the left of the buttons rather than above them */
			m_header.setWidth(LV_SIZE_CONTENT);
			m_buttonPanel.setHeight(LV_SIZE_CONTENT);
		}
	}

	void BabyStep::setBabyStepValue(float value)
	{
		ZoneScoped;
		m_buttonPanel.setResetLabel(_("babystep.reset", value));
	}

	void BabyStep::onShow()
	{
		ZoneScoped;
		const auto values = StorageHelper::getData(ID_BABYSTEP_AMOUNT);
		m_buttonPanel.setIncrementValues(values);
	}
} // namespace UI
