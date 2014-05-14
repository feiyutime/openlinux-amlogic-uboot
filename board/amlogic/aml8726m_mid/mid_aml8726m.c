
#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/aml_card_sd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/am_eth_reg.h>
/**
 * Routine: board_init
 * Description: Early hardware init.
 */
 
#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))
#define delay_us(a) udelay(a)

#define EIO_ID 0x44
 
static void hardi2c_init(void)
{
    /***Clear pinmux***/    
    reg(0xc11080e0) &= ~(1<<31);reg(0xc11080c4) &= ~((1<<16)|(1<<22));reg(0xc11080d0) &= ~(1<<19);//GPIO D12
    
    reg(0xc110804c) &= ~(1<<10);
    reg(0xc1108048) &= ~(1<<10);
    delay_us(20000);
    reg(0xc110804c) |= (1<<10);
    delay_us(20000);
    
    hard_i2c_init(400);
    reg(0xc11080b8) &= ~(0x3f<<0);reg(0xc11080c8) &= ~(3<<28);reg(0xc11080b4) &= ~(1<<31);//AA10 & AB10     GPIOB0(SCL) & GPIOB1(SDA)     HW_I2C_SDA & HW_I2C_SCL
    reg(0xc11080b8) |= ((1<<2) | (1<<5));
    delay_us(10000);
}

static void eio_init(void)
{
    /***output enable***/
    hard_i2c_write8(EIO_ID, 0x0c, 0);
    hard_i2c_write8(EIO_ID, 0x0d, 0);
    hard_i2c_write8(EIO_ID, 0x0e, 0);
}
 
static void power_init(void)
{
    eio_init();
    hard_i2c_write8(EIO_ID, 0x04, 0xfa);//P00 VCC5V_EN & P02 LCD3.3_EN set to low level
    hard_i2c_write8(EIO_ID, 0x05, 0xff);
    hard_i2c_write8(EIO_ID, 0x06, 0xff);
}
 
int board_init(void)
{
    /*
    @todo implement this function
    */
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_arch_number=MACH_TYPE_MESON_8626M;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;
    
    hardi2c_init();
    power_init();
#ifdef CONFIG_CMD_KGDB

	kgdb_init();
#endif

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
/*
    @todo implement this function
    */

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
    /*
    @todo implement this function
    */
}

/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
    /*
    @todo implement this function
    */
	
}

extern int aml_eth_init(bd_t *bis);
int board_eth_init(bd_t *bis)
{
    /*
    @todo implement this function
    */
	///GPIOD15-24 for 8626M;
	///GPIOD12	nRst;
	///GPIOD13    n_int;
	printf("Set 8626m \n");
	//eth_clk_set(ETH_CLKSRC_SYS_D3,900*CLK_1M/3,50*CLK_1M);
	//eth_clk_set(ETH_CLKSRC_SYS_D3,get_cpu_clk()*2/3,50*CLK_1M);	
	eth_clk_set(ETH_CLKSRC_APLL_CLK,400*CLK_1M,50*CLK_1M);
	///GPIOD15-24 for 8626M;
	///GPIOD12	nRst;
	///GPIOD13    n_int;
	//eth_set_pinmux(ETH_BANK2_GPIOD15_D23,ETH_CLK_OUT_GPIOD7_REG4_20,0);
	
	eth_set_pinmux(ETH_BANK2_GPIOD15_D23,ETH_CLK_OUT_GPIOD24_REG5_1,0);
	/*disalbe*/
	//reset:LCD_G5
	writel(readl(ETH_PLL_CNTL) & ~(1 << 0), ETH_PLL_CNTL); // Disable the Ethernet clocks     
	// ---------------------------------------------
	// Test 50Mhz Input Divide by 2
	// ---------------------------------------------
	// Select divide by 2
	writel(readl(ETH_PLL_CNTL) | (0 << 3), ETH_PLL_CNTL); // desc endianess "same order"   
	writel(readl(ETH_PLL_CNTL) | (0 << 2), ETH_PLL_CNTL); // data endianess "little"    
	writel(readl(ETH_PLL_CNTL) | (1 << 1), ETH_PLL_CNTL); // divide by 2 for 100M     
	writel(readl(ETH_PLL_CNTL) | (1 << 0), ETH_PLL_CNTL);  // enable Ethernet clocks   
	udelay(100);
	/*reset*/
	//EIO P13 SET LOW
	
	udelay(100);
	//EIO P13 SET HIGH
	
	//power hold
//	setbits_le32(P_PREG_AGPIO_O,(1<<8));
//	clrbits_le32(P_PREG_AGPIO_EN_N,(1<<8));
	udelay(10);	//waiting reset end;
        aml_eth_init(bis);
        return 0;
}

u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}
static int  sdio_init(unsigned port)
{
	//setbits_le32(P_PREG_CGPIO_EN_N,1<<5);
	setbits_le32(P_PREG_DGPIO_EN_N,1<<11);//GPIOD13

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
	//return (readl(P_PREG_CGPIO_I)&(1<<5))?1:0;
	return (readl(P_PREG_DGPIO_I)&(1<<11))?1:0;//GPIOD13
}
static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
}
static void sdio_pwr_on(unsigned port)
{
//	clrbits_le32(P_PREG_CGPIO_O,(1<<5));
//	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
	clrbits_le32(P_PREG_DGPIO_O,(1<<11));
	clrbits_le32(P_PREG_DGPIO_EN_N,(1<<11));//GPIOD13
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
//	setbits_le32(P_PREG_CGPIO_O,(1<<5));
//	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
	setbits_le32(P_PREG_DGPIO_O,(1<<11));
	clrbits_le32(P_PREG_DGPIO_EN_N,(1<<11));//GPIOD13

	/// @todo NOT FINISH
}
static void board_mmc_register(unsigned port)
{
    AML_CARD_SD_INFO_t *aml_priv=cpu_sdio_get(port);
    if(aml_priv==NULL)
        return;
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 300000;
	mmc->f_min = 200000;
	mmc->f_max = 50000000;
	mmc_register(mmc);
}
int board_mmc_init(bd_t	*bis)
{
	board_mmc_register(SDIO_PORT_A);
//	board_mmc_register(SDIO_PORT_B);
//	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}
