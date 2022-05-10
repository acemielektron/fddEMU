#include "fddEMU.h"
#include "UINotice.h"

class UINotice msg;

void UINotice::show_P(const char* header, const char *notice)
{
#if ENABLE_GUI  
	disp.showNoticeP(header, notice);
#endif //ENABLE_GUI
#if ENABLE_SERIAL 
	Serial.print_P(header);
	Serial.print_P(str_colon);
	Serial.print_P(notice);
	Serial.write('\n');
#endif //ENABLE_SERIAL  
}