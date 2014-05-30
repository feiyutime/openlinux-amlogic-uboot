#include <config.h>
#include <asm/arch/io.h>
void reset_cpu(ulong addr)
{
    setbits_le32(P_WATCHDOG_TC,1<<WATCHDOG_ENABLE_BIT);

#if defined(CONFIG_M6) || defined(CONFIG_M8) || defined (CONFIG_M6TVD)
	//	writel((1<<22) | (3<<24), P_WATCHDOG_TC);
	AML_WATCH_DOG_START();

#endif
    while(1);
}

