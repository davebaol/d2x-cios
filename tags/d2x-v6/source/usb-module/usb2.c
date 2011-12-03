#include <string.h>
#include <stdarg.h>

#include "ipc.h"
#include "mem.h"
#include "module.h"
#include "syscalls.h"
#include "types.h"
#include "usb2.h"

/* Constants */
#define USB_MAX_DEVICES		32

/* USB removal notify */
struct _usb_cb_removalnotify_list {
	usbcallback cb;
	void *userdata;
	s32 device_id;
};

/* USBV5 host structure */
struct _usbv5_host {
	usb_device_entry attached_devices[USB_MAX_DEVICES];
	struct _usb_cb_removalnotify_list remove_cb[USB_MAX_DEVICES];
	s32 fd;
};

/* USB message structure */
struct _usb_msg {
	s32 fd;
	s32 zero;

	union {
		struct {
			u8 bmRequestType;
			u8 bmRequest;
			u16 wValue;
			u16 wIndex;
			u16 wLength;
			void *rpData;
		} ctrl;

		struct {
			void *rpData;
			u16 wLength;
			u8 pad[4];
			u8 bEndpoint;
		} bulk;

		struct {
			void *rpData;
			u16 wLength;
			u8 bEndpoint;
		} intr;

		struct {
			u16 pid;
			u16 vid;
		} notify;

		u8 class;
		u32 hid_intr_write;

		u32 align_pad[4];
	};

        usbcallback cb;
        void *userdata;

	ioctlv vec[7];
};

/* IOCTL commands */
#define USBV5_IOCTL_GETVERSION                   0
#define USBV5_IOCTL_GETDEVICECHANGE              1
#define USBV5_IOCTL_SHUTDOWN                     2
#define USBV5_IOCTL_GETDEVPARAMS                 3
#define USBV5_IOCTL_ATTACHFINISH                 6
#define USBV5_IOCTL_SETALTERNATE                 7
#define USBV5_IOCTL_SUSPEND_RESUME              16
#define USBV5_IOCTL_CANCELENDPOINT              17
#define USBV5_IOCTL_CTRLMSG                     18
#define USBV5_IOCTL_INTRMSG                     19
#define USBV5_IOCTL_ISOMSG                      20
#define USBV5_IOCTL_BULKMSG                     21

/* Variables */
static struct _usbv5_host *host = NULL;


static s32 __usb_control_message(s32 devId, u8 bmRequestType, u8 bmRequest, u16 wValue, u16 wIndex, u16 wLength, void *rpData)
{
	struct _usb_msg *msg;

	u8  adjust = bmRequestType >> 7;
	s32 ret;

	/* Allocate memory */
	msg = Mem_Alloc(sizeof(struct _usb_msg));
	if (!msg)
		return IPC_ENOMEM;

	memset(msg, 0, sizeof(struct _usb_msg));
	msg->fd = devId;
	msg->cb = NULL;
	msg->userdata = NULL;

	/* Setup message */
	msg->ctrl.bmRequestType = bmRequestType;
	msg->ctrl.bmRequest     = bmRequest;
	msg->ctrl.wValue  = wValue;
	msg->ctrl.wIndex  = wIndex;
	msg->ctrl.wLength = wLength;
	msg->ctrl.rpData  = rpData;

	msg->vec[0].data = msg;
	msg->vec[0].len  = 64;
	msg->vec[1].data = rpData;
	msg->vec[1].len  = wLength;

	/* Send control message */
	ret = os_ioctlv(host->fd, USBV5_IOCTL_CTRLMSG, 2 - adjust, adjust, msg->vec);

	/* Free memory */
	Mem_Free(msg);

	return ret;
}

static inline s32 __usb_interrupt_bulk_message(s32 devId, u8 ioctl, u8 bEndpoint, u16 wLength, void *rpData)
{
	struct _usb_msg *msg;

	u8  adjust = bEndpoint >> 7;
	s32 ret;

	if(((s32)rpData%32)!=0) return IPC_EINVAL;
	if(wLength && !rpData) return IPC_EINVAL;
	if(!wLength && rpData) return IPC_EINVAL;

	/* Allocate memory */
	msg = Mem_Alloc(sizeof(struct _usb_msg));
	if (!msg)
		return IPC_ENOMEM;

	memset(msg, 0, sizeof(struct _usb_msg));

	msg->fd = devId;
	msg->cb = NULL;
	msg->userdata = NULL;

	/* Setup message */
	if (ioctl == USBV5_IOCTL_INTRMSG) {
		msg->intr.rpData    = rpData;
		msg->intr.wLength   = wLength;
		msg->intr.bEndpoint = bEndpoint;
	} else {
		msg->bulk.rpData    = rpData;
		msg->bulk.wLength   = wLength;
		msg->bulk.bEndpoint = bEndpoint;
	}

	msg->vec[0].data = msg;
	msg->vec[0].len  = 64;
	msg->vec[1].data = rpData;
	msg->vec[1].len  = wLength;

	/* Send bulk message */
	ret = os_ioctlv(host->fd, ioctl, 2 - adjust, adjust, msg->vec);

	/* Free memory */
	Mem_Free(msg);

	return ret;
}

static s32 __USB_FindDevice(s32 devId)
{
	s32 i;

	/* No host */
	if (!host)
		return IPC_EINVAL;

	/* Find device */
	for (i = 0; host->attached_devices[i].device_id; i++) {
		if (host->attached_devices[i].device_id == devId)
			return i;
	}

	return -1;
}


s32 USB_AttachFinish(s32 result)
{
	/* Wait for device changes (async) */
	if (!result && host)
		os_ioctl_async(host->fd, USBV5_IOCTL_GETDEVICECHANGE, NULL, 0, host->attached_devices, 0x180, queuehandle, MESSAGE_DEVCHANGE);

	return 0;
}

s32 USB_DeviceChange(s32 result)
{
	s32 i, j;

	/* Error */
	if (result < 0 || !host)
		return 0;

	for (i = 0; i < USB_MAX_DEVICES; i++) {
		/* No callback */
		if (!host->remove_cb[i].cb)
			continue;

		/* Check if device is attached */
		for (j = 0; j < result; j++) {
			if (host->remove_cb[i].device_id == host->attached_devices[j].device_id)
				break;
		}

		/* Not attached... remove */
		if (j == result) {
			host->remove_cb[i].cb(0, host->remove_cb[i].userdata);
			host->remove_cb[i].cb = NULL;
		}
	}

	/* Clean free entries */
	memset(host->attached_devices + result, 0, sizeof(usb_device_entry)*(32-result));

	/* Attach finish */
	os_ioctl_async(host->fd, USBV5_IOCTL_ATTACHFINISH, NULL, 0, NULL, 0, queuehandle, MESSAGE_ATTACH);

	return 0;
}

s32 USB_Init(void)
{
	u32 *ver;
	s32  fd, ret;

	/* Already inited */
	if (host)
		return 0;

	/* Open device */
	fd = os_open("/dev/usb/ven", 0);
	if (fd < 0)
		return IPC_ENOENT;

	/* Allocate memory */
	host = Mem_Alloc(sizeof(*host));
	if (!host) {
		/* Close device */
		os_close(fd);

		return IPC_ENOMEM;
	}

	/* Setup buffer */
	memset(host, 0, sizeof(*host));
	host->fd = fd;

	/* Allocate memory */
	ver = Mem_Alloc(32);
	if (!ver)
		return IPC_ENOMEM;

	/* Get version */
	ret = os_ioctl(fd, USBV5_IOCTL_GETVERSION, NULL, 0, ver, 32);

	/* Error or wrong version */
	if (ret || ver[0] != 0x50001) {
		/* Close device */
		os_close(fd);

		/* Free memory */
		Mem_Free(host);
		host = NULL;
	}

	/* Get device changes */
	os_ioctl_async(host->fd, USBV5_IOCTL_GETDEVICECHANGE, NULL, 0, host->attached_devices, 0x180, queuehandle, MESSAGE_DEVCHANGE);

	/* Free memory */
	Mem_Free(ver);

	return ret;
}

void USB_Close(void)
{
	/* Host available */
	if (host) {
		/* Device opened */
		if (host->fd >= 0) {
			/* Shutdown USB2 */
			os_ioctl(host->fd, USBV5_IOCTL_SHUTDOWN, NULL, 0, NULL, 0);

			/* Close device */
			os_close(host->fd);
		}

		/* Free memory */
		Mem_Free(host);
		host = NULL;
	}
}

s32 USB_OpenDevice(s32 devId)
{
	s32 ret;

	/* Find device */
	ret = __USB_FindDevice(devId);
	if (ret < 0)
		return IPC_EINVAL;

	/* Resume device */
	return USB_SuspendResume(devId, 1);
}

s32 USB_CloseDevice(s32 *devId)
{
	s32 i, ret;

	/* Find device */
	ret = __USB_FindDevice(*devId);
	if (ret < 0)
		return IPC_EINVAL;

	/* Remove device */
	for (i = 0; i < USB_MAX_DEVICES; i++) {
		if (!host->remove_cb[i].cb)
			continue;

		if (host->remove_cb[i].device_id == *devId) {
			host->remove_cb[i].cb(0, host->remove_cb[i].userdata);
			host->remove_cb[i].cb = NULL;
			break;
		}
	}

	/* Suspend device */
	ret = USB_SuspendResume(*devId, 0);

	/* Clear descriptor */
	*devId = -1;

	return ret;
}

s32 USB_GetDescriptors(s32 devId, usb_devdesc *udd)
{
	u32 *iobuf  = NULL;
	u8  *buffer = NULL, *next;

	usb_configurationdesc *ucd = NULL;
	usb_interfacedesc     *uid = NULL;
	usb_endpointdesc      *ued = NULL;

	u32 iConf, iEndpoint;
	s32 ret = IPC_ENOMEM;

	/* Find device */
	ret = __USB_FindDevice(devId);
	if (ret < 0)
		return IPC_EINVAL;

	/* Allocate memory */
	iobuf  = Mem_Alloc(0x20);
	buffer = Mem_Alloc(0xC0);

	/* Error */
	if (!iobuf || !buffer)
		goto out;

	/* Setup buffer */
	iobuf[0] = devId;
	iobuf[2] = 0;

	/* Get device parameters */
	ret = os_ioctl(host->fd, USBV5_IOCTL_GETDEVPARAMS, iobuf, 0x20, buffer, 0xC0);
	if (ret)
		goto out;

	next = buffer + 20;

	/* Copy descriptors */
	memcpy(udd, next, sizeof(*udd));

	/* Allocate memory */
	udd->configurations = Mem_Alloc(udd->bNumConfigurations * sizeof(*udd->configurations));
	if (!udd->configurations)
		goto out;

	next += (udd->bLength + 3) & ~3;

	for (iConf = 0; iConf < udd->bNumConfigurations; iConf++) {
		ucd = &udd->configurations[iConf];
		memcpy(ucd, next, USB_DT_CONFIG_SIZE);
		next += (USB_DT_CONFIG_SIZE+3)&~3;

		if (ucd->bNumInterfaces)
			ucd->bNumInterfaces = 1;

		/* Allocate buffer */
		ucd->interfaces = Mem_Alloc(ucd->bNumInterfaces * sizeof(*ucd->interfaces));
		if (!ucd->interfaces)
			goto out;

		uid = ucd->interfaces;
		memcpy(uid, next, USB_DT_INTERFACE_SIZE);
		next += (uid->bLength + 3) & ~3;

		/* Allocate buffer */
		uid->endpoints = Mem_Alloc(uid->bNumEndpoints * sizeof(*uid->endpoints));
		if (!uid->endpoints)
			goto out;

		memset(uid->endpoints,0,uid->bNumEndpoints*sizeof(*uid->endpoints));

		for(iEndpoint = 0; iEndpoint < uid->bNumEndpoints; iEndpoint++) {
			ued = &uid->endpoints[iEndpoint];
			memcpy(ued, next, USB_DT_ENDPOINT_SIZE);
			next += (ued->bLength + 3) & ~3;
		}

		/* Success */
		ret = 0;
	}

out:
	if (iobuf)
		Mem_Free(iobuf);
	if (buffer)
		Mem_Free(buffer);

	return ret;
}

void USB_FreeDescriptors(usb_devdesc *udd)
{
	usb_configurationdesc *ucd;
	usb_interfacedesc     *uid;

	s32 iConf, iInterface;

	if(udd->configurations) {
		for(iConf = 0; iConf < udd->bNumConfigurations; iConf++) {
			ucd = &udd->configurations[iConf];

			if(ucd->interfaces) {
				for(iInterface = 0; iInterface < ucd->bNumInterfaces; iInterface++) {
					uid = &ucd->interfaces[iInterface];
					if(uid->endpoints)
						Mem_Free(uid->endpoints);
					if(uid->extra)
						Mem_Free(uid->extra);
				}

				Mem_Free(ucd->interfaces);
			}
		}

		Mem_Free(udd->configurations);
	}
}

s32 USB_ReadIntrMsg(s32 devId, u8 bEndpoint, u16 wLength, void *rpData)
{
	return __usb_interrupt_bulk_message(devId, USBV5_IOCTL_INTRMSG, bEndpoint, wLength, rpData);
}

s32 USB_WriteIntrMsg(s32 devId, u8 bEndpoint, u16 wLength, void *rpData)
{
	return __usb_interrupt_bulk_message(devId, USBV5_IOCTL_INTRMSG, bEndpoint, wLength, rpData);
}

s32 USB_ReadBlkMsg(s32 devId, u8 bEndpoint, u16 wLength, void *rpData)
{
	return __usb_interrupt_bulk_message(devId, USBV5_IOCTL_BULKMSG, bEndpoint, wLength, rpData);
}

s32 USB_WriteBlkMsg(s32 devId, u8 bEndpoint, u16 wLength, void *rpData)
{
	return __usb_interrupt_bulk_message(devId, USBV5_IOCTL_BULKMSG, bEndpoint, wLength, rpData);
}

s32 USB_ReadCtrlMsg(s32 devId, u8 bmRequestType, u8 bmRequest, u16 wValue, u16 wIndex, u16 wLength, void *rpData)
{
	return __usb_control_message(devId, bmRequestType, bmRequest, wValue, wIndex, wLength, rpData);
}

s32 USB_WriteCtrlMsg(s32 devId, u8 bmRequestType, u8 bmRequest, u16 wValue, u16 wIndex, u16 wLength, void *rpData)
{
	return __usb_control_message(devId, bmRequestType, bmRequest, wValue, wIndex, wLength, rpData);
}

s32 USB_DeviceRemovalNotify(s32 devId, usbcallback cb, void *userdata)
{
	s32 i, ret;

	/* Find device */
	ret = __USB_FindDevice(devId);
	if (ret < 0)
		return IPC_ENOENT;

	/* Check if already registered */
	for (i = 0; i < USB_MAX_DEVICES; i++) {
		if (host->remove_cb[i].cb && host->remove_cb[i].device_id == devId)
			return IPC_EINVAL;
	}

	/* Find a free entry */
	for (i = 0; i < USB_MAX_DEVICES; i++) {
		if (!host->remove_cb[i].cb) {
			host->remove_cb[i].cb = cb;
			host->remove_cb[i].userdata  = userdata;
			host->remove_cb[i].device_id = devId;
			return 0;
		}
	}

	return IPC_EINVAL;
}

s32 USB_SuspendResume(s32 devId, s32 resumed)
{
	s32 *buf;
	s32  ret;

	/* Find device */
	ret = __USB_FindDevice(devId);
	if (ret < 0)
		return IPC_ENOENT;

	/* Allocate memory */
	buf = Mem_Alloc(32);
	if (!buf)
		return IPC_ENOMEM;

	memset(buf, 0, 32);

	/* Setup buffer */
	buf[0] = devId;
	buf[2] = resumed;

	/* Suspend/Resume device */
	ret = os_ioctl(host->fd, USBV5_IOCTL_SUSPEND_RESUME, buf, 32, NULL, 0);

	/* Free memory */
	Mem_Free(buf);

	return ret;
}

s32 USB_GetDeviceList(usb_device_entry *buffer, u8 num_descr, u8 interface, u8 *cnt_descr)
{
	u32 cnt = 0, i = 0;

	/* No support for HID devices */
	if (interface == USB_CLASS_HID)
		return IPC_EINVAL;

	/* Get devices */
	while (cnt < num_descr && host->attached_devices[i].device_id) {
		/* Copy device entry */
		buffer[cnt++] = host->attached_devices[i++];

		/* Check limit */
		if (i >= USB_MAX_DEVICES)
			break;
	}

	/* Copy number of devices */
	if (cnt_descr)
		*cnt_descr = cnt;

	return 0;
}

s32 USB_SetConfiguration(s32 fd, u8 configuration)
{
	/* Send control message */
	return __usb_control_message(fd, (USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_DEVICE), USB_REQ_SETCONFIG, configuration, 0, 0, NULL);
}

s32 USB_GetConfiguration(s32 fd, u8 *configuration)
{
	u8 *conf;
	s32 ret;

	/* Allocate memory */
	conf = Mem_Alloc(1);
	if(!conf)
		return IPC_ENOMEM;

	/* Send control message */
	ret = __usb_control_message(fd, (USB_CTRLTYPE_DIR_DEVICE2HOST | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_DEVICE), USB_REQ_GETCONFIG, 0, 0, 1, conf);
	if (ret >= 0)
		*configuration = *conf;

	/* Free memory */
	Mem_Free(conf);

	return ret;
}

s32 USB_SetAlternativeInterface(s32 fd, u8 interface, u8 alternateSetting)
{
	if (!alternateSetting)
		return IPC_EINVAL;

	/* Send control message */
	return __usb_control_message(fd, (USB_CTRLTYPE_DIR_HOST2DEVICE | USB_CTRLTYPE_TYPE_STANDARD | USB_CTRLTYPE_REC_INTERFACE), USB_REQ_SETINTERFACE, alternateSetting, interface, 0, NULL);
}

s32 USB_CancelEndpoint(s32 devId, u8 endpoint)
{
	s32 *buf;
	s32  ret;

	/* Find device */
	ret = __USB_FindDevice(devId);
	if (ret < 0)
		return IPC_ENOENT;

	/* Allocate memory */
	buf = Mem_Alloc(32);
	if (!buf)
		return IPC_ENOMEM;

	memset(buf, 0, 32);

	/* Setup buffer */
	buf[0] = devId;
	buf[2] = endpoint;

	/* Cancel end point */
	ret = os_ioctl(host->fd, USBV5_IOCTL_CANCELENDPOINT, buf, 32, NULL, 0);

	/* Free memory */
	Mem_Free(buf);

	return ret;
}
