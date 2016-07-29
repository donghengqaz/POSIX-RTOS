#include "eth.h"

#include "stm32_eth.h"
#include "io.h"

#include "string.h"
#include "ipport.h"
#include "ippkg.h"

#define ETH_RXBUFNB (4)
#define ETH_TXBUFNB (2)
#define ETH_DMARxDesc_FrameLengthShift (16)


#define PHY_ADDRESS (0x01)

static os_u8 eth_phy_rx_buffer[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], eth_phy_tx_buffer[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];
static ETH_DMADESCTypeDef eth_phy_rx_dscr_tab[ETH_RXBUFNB], eth_phy_tx_dscr_tab[ETH_TXBUFNB];
extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMADESCTypeDef *DMARxDescToGet;
ETH_DMADESCTypeDef  *DMATxDesc = eth_phy_tx_dscr_tab;
ETH_DMADESCTypeDef  *DMARxDesc = eth_phy_rx_dscr_tab;

static struct ipport ipport;

/******************************************************************************/

static void eth_phy_clk_init(void)
{
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx | RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD 
                            | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
}

static void eth_phy_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void eth_phy_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_Remap_ETH, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void
eth_phy_mco_init(void)
{
  GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
  RCC_PLL3Config(RCC_PLL3Mul_10);
  RCC_PLL3Cmd(ENABLE);
  while( RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET ){ }
  RCC_MCOConfig(RCC_MCO_PLL3CLK);   
}

static void
eth_phy_confugration(void)
{
  ETH_InitTypeDef ETH_InitStructure;
  
  ETH_DeInit();
  ETH_SoftwareReset();
  while( ETH_GetSoftwareResetStatus() == SET ){ }
  
  ETH_StructInit(&ETH_InitStructure);
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable  ;
  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
  ETH_Init(&ETH_InitStructure, PHY_ADDRESS);

  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
}

/**********************************************************************************************/

void 
stm32f107_eth_init(void)
{
  eth_phy_clk_init();
  eth_phy_nvic_init();
  eth_phy_gpio_init();
  eth_phy_mco_init();
  eth_phy_confugration();
}

/**********************************************************************************************/


/**********************************************************************************************/

static err_t eth_init(void)
{
  ETH_DMATxDescChainInit(eth_phy_tx_dscr_tab, &eth_phy_tx_buffer[0][0], ETH_TXBUFNB);
  ETH_DMARxDescChainInit(eth_phy_rx_dscr_tab, &eth_phy_rx_buffer[0][0], ETH_RXBUFNB);

	{
		int i;

		for( i = 0; i < ETH_RXBUFNB; i++)
        {
			ETH_DMARxDescReceiveITConfig(&eth_phy_rx_dscr_tab[i], ENABLE);
		}
	}
#ifdef CHECKSUM_BY_HARDWARE
	{
		int i;

		for( i = 0; i < ETH_TXBUFNB; i++)
        {
			ETH_DMATxDescChecksumInsertionConfig(&eth_phy_tx_dscr_tab[i], ETH_DMATxDesc_ChecksumTCPUDPICMPFull);
		}
	}
#endif
	ETH_Start();
    
    return 0;
}

/**********************************************************************************************/

static err_t eth_tx_pkg(void *pkg)
{
	os_u32 len;
	os_u8 *buffer =  (os_u8 *)DMATxDescToSet->Buffer1Addr;

    ippkg_unpack(pkg, buffer, &len);

	if( (DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET ){
		return ERR_IF;
	}

	DMATxDescToSet->ControlBufferSize = (len & ETH_DMATxDesc_TBS1);
	DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
	DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

	if ( (ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET )
    {
		ETH->DMASR = ETH_DMASR_TBUS;
		ETH->DMATPDR = 0;
	}

	DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);

	return ERR_OK;
}

/**********************************************************************************************/

static void* eth_rx_pkg(void)
{
	int frame_len;
    os_u8 *frame_buffer;
	ETH_DMADESCTypeDef *rx_dscr;
    void *p;

	if((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (u32)RESET)
    {
	     if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
         {
	       ETH->DMASR = ETH_DMASR_RBUS;
	       ETH->DMARPDR = 0;
	     }
	     return 0;
	}

	if( ((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (u32)RESET)
			&& ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (u32)RESET)
			&& ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (u32)RESET) )
    {
		frame_len    = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
		frame_buffer = (os_u8 *)DMARxDescToGet->Buffer1Addr;
	}
    else
    {
		frame_len = 0;
	}

	rx_dscr = DMARxDescToGet;

	DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);

	p = ippkg_pack(frame_buffer, frame_len);

	rx_dscr->Status = ETH_DMARxDesc_OWN;

	if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET){
		ETH->DMASR = ETH_DMASR_RBUS;
	    ETH->DMARPDR = 0;
	}

	return p;
}

/**********************************************************************************************/

void set_mac_address( const char *mac_addr )
{
  ipport.mac_addr[0] = mac_addr[0];
  ipport.mac_addr[1] = mac_addr[1];
  ipport.mac_addr[2] = mac_addr[2];
  ipport.mac_addr[3] = mac_addr[3];
  ipport.mac_addr[4] = mac_addr[4];
  ipport.mac_addr[5] = mac_addr[5];
  
  ETH_MACAddressConfig(ETH_MAC_Address0, ipport.mac_addr);  
}

void ETH_IRQHandler(void)
{ 
    register int status = ETH->DMASR;
  
    if( (status & ETH_DMA_IT_R) != (u32)RESET )
    {      
       ETH_DMAClearITPendingBit( ETH_DMA_IT_R );
       
       mailbox_send( &ipport.rx_mailbox, 0 );
    }
  
    if ((status & ETH_DMA_IT_NIS) != (u32)RESET)
    {
        ETH->DMASR = (u32)ETH_DMA_IT_NIS;
    }
}

err_t eth_configuration(void)
{
  const char mac_address[6] = {0x0a,1,2,3,2,1};

  stm32f107_eth_init();

#if LWIP_DHCP
  ipport.ipaddr.addr  = 0;
  ipport.netmask.addr = 0;
  ipport.gw.addr      = 0;
#else
  
#if 0
  IP4_ADDR(&ipport.ipaddr,  192, 168,   0, 180 );
  IP4_ADDR(&ipport.netmask, 255, 255, 255,   0 );
  IP4_ADDR(&ipport.gw,      192, 168,   0,   1 );  
#endif
  
#endif
  set_mac_address(mac_address);

  ipport.hal_rx   = eth_rx_pkg;
  ipport.hal_tx   = eth_tx_pkg;
  ipport.hal_init = eth_init;
  
  ipport_create( &ipport );

  return 0;  
}

/******************************************************************************/
