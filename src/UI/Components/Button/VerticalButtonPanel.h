/*
 * VerticalButtonPanel.h
 *
 *  Created on: 2025-02-28
 *      Author: Andy Everitt
 */

#pragma once

#include "UI/Components/Button/Button.h"
#include "UI/Components/Input/ModalNumberPad.h"
#include "UI/Components/List/List.h"
#include "UI/Core/View.h"
#include <span>

namespace UI
{
	class VerticalButtonPanel : public LvObj
	{
	  public:
		VerticalButtonPanel(const std::string& name, LvObj& parent);
		void setIncrementIcon(std::string_view icon);
		void setDecrementIcon(std::string_view icon);
		void setIncrementLabel(std::string_view label);
		void setDecrementLabel(std::string_view label);
		void setResetLabel(std::string_view label);
		void setValueLabelFmt(std::string_view fmt);
		void setIncrementValues(std::span<const float> values);
		void setDisabled(bool disabled);
		// Lays the buttons out in a row instead of a column, for use in a wide, short space.
		void setHorizontal(bool horizontal);

		float getSelectedValue() const;
		void setSelectedValueIndex(size_t index);

		void setMinValue(float min) { m_minValue = min; }
		void setMaxValue(float max) { m_maxValue = max; }

		void setValueChangeCallback(std::function<void(float)> callback);
		void setResetCallback(std::function<void()> callback);
		void setUpdatedValuesCallback(std::function<void(const std::vector<float>&)> callback);

		Button& getResetButton() { return m_reset; }
		Button& getIncrementButton() { return m_increment; }
		Button& getDecrementButton() { return m_decrement; }

		void setNumberPad(ModalNumberPad* numberPad) { m_numberPad = numberPad; }
		ModalNumberPad* getNumberPad() const { return m_numberPad; }

	  private:
		std::unique_ptr<Button> createValueButton(size_t index, LvObj& parent);

		void updateValueLabels();

		Button m_reset;
		Button m_increment;
		Button m_decrement;

		List<Button> m_values;

		ModalNumberPad* m_numberPad = nullptr;
		bool m_horizontal = false;

		size_t m_selectedValueIndex = 0;

		std::string m_fmt;
		std::vector<float> m_incrementValues;

		float m_minValue = 0.0f;
		float m_maxValue = 100.0f;

		std::function<void(float)> m_valueChangeCallback; // Called when increment/decrement is pressed
		std::function<void()> m_resetCallback;			  // Called when reset is pressed
		std::function<void(const std::vector<float>&)>
			m_updatedValuesCallback; // Called when increment values are updated by long press
	};
} // namespace UI
