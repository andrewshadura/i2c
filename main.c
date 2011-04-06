typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8   uint8;

/* General Defines */
#define MMIO_SIZE                   (512*1024)

/* I/O Control Registers (05000h 05FFFh) */
#define HVSYNC                0x05000 
#define GPIOA                 0x05010	//	"I810-DDC"
#define GPIOB                 0x05014	//	"I810-I2C" 
#define GPIOC                 0x0501C	//	"I810-GPIOC"

/* 
 * Register I/O
 */
#define i810_readb(where, mmio) readb(mmio + where)
#define i810_readw(where, mmio) readw(mmio + where)
#define i810_readl(where, mmio) readl(mmio + where)
#define i810_writeb(where, mmio, val) writeb(val, mmio + where) 
#define i810_writew(where, mmio, val) writew(val, mmio + where)
#define i810_writel(where, mmio, val) writel(val, mmio + where)

/* bit locations in the registers */
#define SCL_DIR_MASK	0x0001
#define SCL_DIR			0x0002
#define SCL_VAL_MASK	0x0004
#define SCL_VAL_OUT		0x0008
#define SCL_VAL_IN		0x0010
#define SDA_DIR_MASK	0x0100
#define SDA_DIR			0x0200
#define SDA_VAL_MASK	0x0400
#define SDA_VAL_OUT		0x0800
#define SDA_VAL_IN		0x1000

unsigned long ddc_base = GPIOB;
unsigned long mmio_start_phys;

//par->mmio_start_phys = pci_resource_start(par->dev, 1);
//par->mmio_start_virtual = ioremap_nocache(par->mmio_start_phys, MMIO_SIZE);

static void i810i2c_setscl(void *data, int state)
{
	struct i810fb_i2c_chan    *chan = data;
	struct i810fb_par         *par = chan->par;
	uint8                        __iomem *mmio = par->mmio_start_virtual;

	if (state)
		i810_writel(mmio, ddc_base, SCL_DIR_MASK | SCL_VAL_MASK);
	else
		i810_writel(mmio, ddc_base, SCL_DIR | SCL_DIR_MASK | SCL_VAL_MASK);
	i810_readl(mmio, chan->ddc_base);	/* flush posted write */
}

static void i810i2c_setsda(void *data, int state)
{
	struct i810fb_i2c_chan    *chan = data;
	struct i810fb_par         *par = chan->par;
	uint8                        __iomem *mmio = par->mmio_start_virtual;

	if (state)
		i810_writel(mmio, ddc_base, SDA_DIR_MASK | SDA_VAL_MASK);
	else
		i810_writel(mmio, ddc_base, SDA_DIR | SDA_DIR_MASK | SDA_VAL_MASK);
	i810_readl(mmio, chan->ddc_base);	/* flush posted write */
}

static int i810i2c_getscl(void *data)
{
	struct i810fb_i2c_chan    *chan = data;
	struct i810fb_par         *par = chan->par;
	uint8                        __iomem *mmio = par->mmio_start_virtual;

	i810_writel(mmio, ddc_base, SCL_DIR_MASK);
	i810_writel(mmio, dcd_base, 0);
	return ((i810_readl(mmio, ddc_base) & SCL_VAL_IN) != 0);
}

static int i810i2c_getsda(void *data)
{
	struct i810fb_i2c_chan    *chan = data;
	struct i810fb_par         *par = chan->par;
	uint8                        __iomem *mmio = par->mmio_start_virtual;

	i810_writel(mmio, ddc_base, SDA_DIR_MASK);
	i810_writel(mmio, ddc_base, 0);
	return ((i810_readl(mmio, ddc_base) & SDA_VAL_IN) != 0);
}

int _tmain(int argc, char* argv[])
{
	return 0;
}

