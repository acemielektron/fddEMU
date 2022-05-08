#include "fddEMU.h"
#include "UINotice.h"
#include "gui/GraphicUI.h" //disp
#include "serial/simpleUART.h" //Serial

class UINotice msg;

void UINotice::show_P(const char* header, const char *notice)
{
#if ENABLE_GUI  
	disp.showNoticeP(header, notice);
#endif //ENABLE_GUI
#if ENABLE_SERIAL || DEBUG  
	Serial.print_P(header);
	Serial.print_P(str_colon);
	Serial.print_P(notice);
	Serial.write('\n');
#endif //ENABLE_SERIAL || DEBUG  
}