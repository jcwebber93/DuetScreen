/*
 * BabyStep.h
 *
 *  Created on: 2025-09-25
 *      Author: Andy Everitt
 */

#pragma once

#include "BabyStepPresenter.h"
#include "UI/Components/Button/VerticalButtonPanel.h"
#include "UI/Core/View.h"

namespace UI
{
	class BabyStep : public View<BabyStepPresenter>
	{
	  public:
		BabyStep(const std::string& name, LvObj& parent);

		void setBabyStepValue(float value);
		void setNumberPad(ModalNumberPad* numberPad) { m_buttonPanel.setNumberPad(numberPad); }
		void setDisabled(bool disabled) { m_buttonPanel.setDisabled(disabled); }
		// Lays the control out as a wide, short bar instead of a tall column.
		void setHorizontal(bool horizontal);

	  protected:
		void onShow() override;

	  private:
		LvLabel m_header{"header", getRoot()};
		VerticalButtonPanel m_buttonPanel{"button_panel", getRoot()};
	};
} // namespace UI
