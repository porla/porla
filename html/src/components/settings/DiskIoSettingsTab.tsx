import { Checkbox } from "@chakra-ui/checkbox";
import { FormControl, FormErrorMessage, FormHelperText, FormLabel } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Code, Text } from "@chakra-ui/layout";
import { Select } from "@chakra-ui/select";
import { Field, useFormikContext } from "formik";

type IFormData = {
  disk_write_mode: number;
  file_pool_size: number;
  hashing_threads: number;
  max_queued_disk_bytes: number;
  mmap_file_size_cutoff: number;
  no_atime_storage: boolean;
  send_buffer_watermark: number;
  send_buffer_watermark_factor: number;
  send_buffer_low_watermark: number;
}

export default function DiskIoSettingsTab() {
  const {
    errors,
    handleChange,
    touched,
    setFieldValue,
    values
  } = useFormikContext<IFormData>();

  return (
    <Box>
      <Field name="file_pool_size">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>File pool size</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.file_pool_size && touched.file_pool_size
              ? <FormErrorMessage>{errors.file_pool_size}</FormErrorMessage>
              : <FormHelperText>The upper limit on the total number of files this session will keep open. The reason why files are left open at all is that some anti virus software hooks on every file close, and scans the file for viruses. deferring the closing of the files will be the difference between a usable system and a completely hogged down system. Most operating systems also has a limit on the total number of file descriptors a process may have open.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="send_buffer_watermark">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Send buffer watermark (bytes)</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.send_buffer_watermark && touched.send_buffer_watermark
              ? <FormErrorMessage>{errors.send_buffer_watermark}</FormErrorMessage>
              : <FormHelperText>If the send buffer has fewer bytes than <Code>send_buffer_watermark</Code>, we'll read another 16 kiB block onto it. If set too small, upload rate capacity will suffer. If set too high, memory will be wasted. The actual watermark may be lower than this in case the upload rate is low, this is the upper limit.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="send_buffer_watermark_factor">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Send buffer watermark factor</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.send_buffer_watermark_factor && touched.send_buffer_watermark_factor
              ? <FormErrorMessage>{errors.send_buffer_watermark_factor}</FormErrorMessage>
              : <FormHelperText>The current upload rate to a peer is multiplied by this factor to get the send buffer watermark. The factor is specified as a percentage. i.e. 50 -&gt; 0.5 This product is clamped to the <Code>send_buffer_watermark</Code> setting to not exceed the max. For high speed upload, this should be set to a greater value than 100. For high capacity connections, setting this higher can improve upload performance and disk throughput. Setting it too high may waste RAM and create a bias towards read jobs over write jobs.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="send_buffer_low_watermark">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Send buffer low watermark (bytes)</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.send_buffer_low_watermark && touched.send_buffer_low_watermark
              ? <FormErrorMessage>{errors.send_buffer_low_watermark}</FormErrorMessage>
              : <FormHelperText>The minimum send buffer target size (send buffer includes bytes pending being read from disk). For good and snappy seeding performance, set this fairly high, to at least fit a few blocks. This is essentially the initial window size which will determine how fast we can ramp up the send rate.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="hashing_threads">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Hashing threads</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.hashing_threads && touched.hashing_threads
              ? <FormErrorMessage>{errors.hashing_threads}</FormErrorMessage>
              : <FormHelperText>
                <Text mb={2}>The number of disk I/O threads to use for
                piece hash verification. These threads are *in addition* to the
                regular disk I/O threads specified by settings_pack::aio_threads.
                These threads are only used for full checking of torrents. The
                hash checking done while downloading are done by the regular disk
                I/O threads.</Text>
                <Text>The hasher threads do not only compute hashes, but also perform
                the read from disk. On storage optimal for sequential access,
                such as hard drives, this setting should be set to 1, which is
                also the default.</Text>
              </FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="disk_write_mode">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Disk write mode</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("disk_write_mode", parseInt(e.target.value))
              }}
            >
              <option value={0}>always_pwrite</option>
              <option value={1}>always_mmap_write</option>
              <option value={2}>auto_mmap_write</option>
            </Select>
            {
              errors.disk_write_mode && touched.disk_write_mode
              ? <FormErrorMessage>{errors.disk_write_mode}</FormErrorMessage>
              : <FormHelperText>Controls whether disk writes will be made through a memory mapped file or via normal write calls. This only affects the mmap_disk_io. When saving to a non-local drive (network share, NFS or NAS) using memory mapped files is most likely inferior. When writing to a local SSD (especially in DAX mode) using memory mapped files likely gives the best performance.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="mmap_file_size_cutoff">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Memory-mapped file size cutoff</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.mmap_file_size_cutoff && touched.mmap_file_size_cutoff
              ? <FormErrorMessage>{errors.mmap_file_size_cutoff}</FormErrorMessage>
              : <FormHelperText>When using mmap_disk_io, files smaller than this number of blocks will not be memory mapped, but will use normal pread/pwrite operations. This file size limit is specified in 16 kiB blocks.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="no_atime_storage">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel><Code>O_NOATIME</Code> storage</FormLabel>
            <Checkbox
              id={"no_atime_storage"}
              name={"no_atime_storage"}
              onChange={handleChange}
              isChecked={values.no_atime_storage}
            />
            {
              errors.no_atime_storage && touched.no_atime_storage
                ? <FormErrorMessage>{errors.no_atime_storage}</FormErrorMessage>
                : <FormHelperText>This is a Linux-only option and passes in the <Code>O_NOATIME</Code> to <Code>open()</Code> when opening files. This may lead to some disk performance improvements.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_queued_disk_bytes">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max queued disk bytes</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_queued_disk_bytes && touched.max_queued_disk_bytes
              ? <FormErrorMessage>{errors.max_queued_disk_bytes}</FormErrorMessage>
              : <FormHelperText>The maximum number of bytes, to be written to disk, that can wait in the disk I/O thread queue. This queue is only for waiting for the disk I/O thread to receive the job and either write it to disk or insert it in the write cache. When this limit is reached, the peer connections will stop reading data from their sockets, until the disk thread catches up. Setting this too low will severely limit your download rate.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
    </Box>
  )
}
