#include "libfm.h"

int main()
{
	fm_add_hook("load_msg");
	fm_init();
	fm_stop();
}
