/*
 * VerticalButtonPanel.cpp
 *
 *  Created on: 2025-02-28
 *      Author: Andy Everitt
 */

#include "VerticalButtonPanel.h"
#include "Debug.h"
#include "UI/Styles/Styles.h"
#include "i18n/i18n.h"

namespace UI
{
	VerticalButtonPanel::VerticalButtonPanel(const std::string& name, LvObj& parent)
		: LvObj(lv_obj_create, name, parent)
		, m_reset("reset", getRoot())
		, m_increment("increment", getRoot())
		, m_decrement("decrement", getRoot())
		, m_values{"value_list", getRoot()}
	{
		ZoneScoped;
		UI_LOCK();
		setFlexFlow(LV_FLEX_FLOW_COLUMN);
		setFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

		iterateChildren([](size_t /* i */, LvObj& child) { child.setWidth(LV_PCT(100)); });

		m_reset.setFlexGrow(1);
		m_reset.setMinHeight(LV_SIZE_CONTENT);
		m_increment.setFlexGrow(3);
		m_decrement.setFlexGrow(3);

		m_values.setFlexGrow(2);
		m_values.setMinHeight(LV_SIZE_CONTENT);
		m_values.setListFlow(LV_FLEX_FLOW_ROW);
		m_values.getListContainer().setFlexAlign(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
		m_values.getListContainer().setFlexGrow(1);
		// m_values.getListContainer().setHeight(LV_PCT(100));
		m_values.getListContainer().setMinHeight(LV_SIZE_CONTENT);
		m_values.setStylePad(0);

		m_increment.setIcon("increment.png");
		m_decrement.setIcon("decrement.png");

		m_decrement.addClickedCallback(
			[](lv_event_t* e)
			{
				UI_LOCK();
				VerticalButtonPanel* panel = static_cast<VerticalButtonPanel*>(lv_event_get_user_data(e));
				if (panel->m_valueChangeCallback)
				{
					panel->m_valueChangeCallback(-panel->getSelectedValue());
				}
			},
			this);

		m_increment.addClickedCallback(
			[](lv_event_t* e)
			{
				UI_LOCK();
				VerticalButtonPanel* panel = static_cast<VerticalButtonPanel*>(lv_event_get_user_data(e));
				if (panel->m_valueChangeCallback)
				{
					panel->m_valueChangeCallback(panel->getSelectedValue());
				}
			},
			this);

		m_reset.addClickedCallback(
			[](lv_event_t* e)
			{
				UI_LOCK();
				VerticalButtonPanel* panel = static_cast<VerticalButtonPanel*>(lv_event_get_user_data(e));
				if (panel->m_resetCallback)
				{
					panel->m_resetCallback();
				}
			},
			this);
	}

	void VerticalButtonPanel::setHorizontal(bool horizontal)
	{
		ZoneScoped;
		UI_LOCK();
		m_horizontal = horizontal;
		setFlexFlow(horizontal ? LV_FLEX_FLOW_ROW : LV_FLEX_FLOW_COLUMN);
		/* The children stretch along the flex axis and fill the container on the cross axis, so the two sizes
		 * swap over when the flow does. */
		iterateChildren(
			[horizontal](size_t /* i */, LvObj& child)
			{
				child.setWidth(horizontal ? LV_SIZE_CONTENT : LV_PCT(100));
				child.setHeight(horizontal ? LV_PCT(100) : LV_SIZE_CONTENT);
			});

		/* In a row the text-bearing children size to their content and the arrows absorb the slack. Letting the
		 * text grow instead just clips the labels, since growing overrides the content width. */
		m_reset.setFlexGrow(horizontal ? 0 : 1);
		m_values.setFlexGrow(horizontal ? 0 : 2);
		if (horizontal)
		{
			m_reset.setWidth(LV_SIZE_CONTENT);
			m_values.setWidth(LV_SIZE_CONTENT);
			m_values.setListSize(LV_SIZE_CONTENT, LV_PCT(100));
		}
	}

	void VerticalButtonPanel::setIncrementIcon(std::string_view icon)
	{
		ZoneScoped;
		m_increment.setIcon(icon);
	}

	void VerticalButtonPanel::setDecrementIcon(std::string_view icon)
	{
		ZoneScoped;
		m_decrement.setIcon(icon);
	}

	void VerticalButtonPanel::setIncrementLabel(std::string_view label)
	{
		ZoneScoped;
		m_increment.setText(label);
	}

	void VerticalButtonPanel::setDecrementLabel(std::string_view label)
	{
		ZoneScoped;
		m_decrement.setText(label);
	}

	void VerticalButtonPanel::setResetLabel(std::string_view label)
	{
		ZoneScoped;
		m_reset.setText(label);
	}

	void VerticalButtonPanel::setValueLabelFmt(std::string_view fmt)
	{
		ZoneScoped;
		UI_LOCK();
		m_fmt = fmt;

		updateValueLabels();
	}

	void VerticalButtonPanel::setIncrementValues(std::span<const float> values)
	{
		ZoneScoped;
		UI_LOCK();
		m_values.setItemCount(values.size(), this, &VerticalButtonPanel::createValueButton);
		m_incrementValues.clear();
		m_incrementValues.insert(m_incrementValues.end(), values.begin(), values.end());

		updateValueLabels();
	}

	void VerticalButtonPanel::setDisabled(bool disabled)
	{
		ZoneScoped;
		UI_LOCK();
		m_increment.setDisabled(disabled);
		m_decrement.setDisabled(disabled);
		m_reset.setDisabled(disabled);
	}

	std::unique_ptr<Button> VerticalButtonPanel::createValueButton(size_t index, LvObj& parent)
	{
		ZoneScoped;
		auto btn = std::make_unique<Button>(fmt::format("value_btn_{}", index), parent);
		btn->getLabel().setLongMode(LV_LABEL_LONG_MODE_WRAP);
		/* Width 0 lets the label wrap, which is what the tall column needs; the wide bar has room for the
		 * value on one line instead. */
		btn->setSize(m_horizontal ? LV_SIZE_CONTENT : 0, LV_PCT(100));
		btn->setMinHeight(LV_SIZE_CONTENT); // FIXME: this seems to cause a lvgl layout bug
		/* Growing would override the content width and clip the label */
		btn->setFlexGrow(m_horizontal ? 0 : 1);
		btn->setCheckable(true);
		btn->setChecked(index == m_selectedValueIndex);
		btn->setUserData(reinterpret_cast<void*>(static_cast<uintptr_t>(index)));
		btn->addClickedCallback([this, index](lv_event_t*) { setSelectedValueIndex(index); });
		btn->addEventCallback(
			[this, btnPtr = btn.get(), index](lv_event_t*)
			{
				auto np = getNumberPad();
				if (np == nullptr)
				{
					LOG_DBG("No numberpad set");
					return;
				}

				if (index >= m_incrementValues.size())
					return;

				float value = m_incrementValues.at(index);
				openModal(np);
				np->setHeader(_("multi_value_selector.numberpad_new_value_header"));
				np->setValue(value);
				np->setMinValue(m_minValue);
				np->setMaxValue(m_maxValue);
				np->setConfirmCallback(
					[this, index](float value)
					{
						m_incrementValues.at(index) = value;
						updateValueLabels();
						if (m_updatedValuesCallback)
						{
							m_updatedValuesCallback(m_incrementValues);
						}
					});
			},
			LV_EVENT_LONG_PRESSED);
		return btn;
	}

	void VerticalButtonPanel::updateValueLabels()
	{
		ZoneScoped;
		m_values.iterateListItems([this](size_t index, Button& btn)
								  { btn.setText(fmt::format(fmt::runtime(m_fmt), m_incrementValues.at(index))); });
	}

	float VerticalButtonPanel::getSelectedValue() const
	{
		ZoneScoped;
		return m_incrementValues[m_selectedValueIndex];
	}

	void VerticalButtonPanel::setSelectedValueIndex(size_t index)
	{
		ZoneScoped;
		UI_LOCK();
		m_values.iterateListItems([index](size_t i, Button& btn) { btn.setChecked(i == index); });
		m_selectedValueIndex = index;
	}

	void VerticalButtonPanel::setValueChangeCallback(std::function<void(float)> callback)
	{
		ZoneScoped;
		UI_LOCK();
		m_valueChangeCallback = std::move(callback);
	}

	void VerticalButtonPanel::setResetCallback(std::function<void()> callback)
	{
		ZoneScoped;
		UI_LOCK();
		m_resetCallback = std::move(callback);
	}

	void VerticalButtonPanel::setUpdatedValuesCallback(std::function<void(const std::vector<float>&)> callback)
	{
		ZoneScoped;
		UI_LOCK();
		m_updatedValuesCallback = std::move(callback);
	}
} // namespace UI
