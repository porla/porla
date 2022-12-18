import { Checkbox } from "@chakra-ui/checkbox";
import { FormControl, FormErrorMessage, FormHelperText, FormLabel } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Code } from "@chakra-ui/layout";
import { Field, useFormikContext } from "formik";

type IFormData = {
  active_checking: number;
  active_downloads: number;
  active_limit: number;
  active_seeds: number;
  auto_manage_interval: number;
  auto_manage_prefer_seeds: boolean;
  auto_scrape_interval: number;
  auto_scrape_min_interval: number;
  dont_count_slow_torrents: boolean;
  inactive_down_rate: number;
  inactive_up_rate: number;
  incoming_starts_queued_torrents: boolean;
}

export default function QueueingSettingsTab() {
  const {
    errors,
    handleChange,
    touched,
    values
  } = useFormikContext<IFormData>();

  return (
    <Box>
      <Field name="active_limit">
        {(w: any) => (
          <FormControl isInvalid={!!errors.active_limit && touched.active_limit} mb={3}>
            <FormLabel>Active limit</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.active_limit && touched.active_limit
                ? <FormErrorMessage>{errors.active_limit}</FormErrorMessage>
                : <FormHelperText>A hard limit on the number of active (auto managed) torrents. This limit also applies to slow torrents.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="active_downloads">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Active downloads</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.active_downloads && touched.active_downloads
                ? <FormErrorMessage>{errors.active_downloads}</FormErrorMessage>
                : <FormHelperText>The upper limit on the number of downloading torrents. Setting the value to -1 means unlimited.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="active_seeds">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Active seeds</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.active_seeds && touched.active_seeds
                ? <FormErrorMessage>{errors.active_seeds}</FormErrorMessage>
                : <FormHelperText>The upper limit on the number of seeding torrents. Setting the value to -1 means unlimited.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="active_checking">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Active checking</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.active_checking && touched.active_checking
                ? <FormErrorMessage>{errors.active_checking}</FormErrorMessage>
                : <FormHelperText>The limit of number of simultaneous checking torrents.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="auto_manage_interval">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Auto manage interval</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.auto_manage_interval && touched.auto_manage_interval
                ? <FormErrorMessage>{errors.auto_manage_interval}</FormErrorMessage>
                : <FormHelperText>The number of seconds between the torrent queue is updated, and rotated.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="auto_scrape_interval">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Auto scrape interval</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.auto_scrape_interval && touched.auto_scrape_interval
                ? <FormErrorMessage>{errors.auto_scrape_interval}</FormErrorMessage>
                : <FormHelperText>The number of seconds between scrapes of queued torrents (auto managed and paused torrents). Auto managed torrents that are paused, are scraped regularly in order to keep track of their downloader/seed ratio. This ratio is used to determine which torrents to seed and which to pause.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="auto_scrape_min_interval">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Auto scrape min interval</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.auto_scrape_min_interval && touched.auto_scrape_min_interval
                ? <FormErrorMessage>{errors.auto_scrape_min_interval}</FormErrorMessage>
                : <FormHelperText>The minimum number of seconds between any automatic scrape (regardless of torrent). In case there are a large number of paused auto managed torrents, this puts a limit on how often a scrape request is sent.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="inactive_down_rate">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Inactive download rate</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.inactive_down_rate && touched.inactive_down_rate
                ? <FormErrorMessage>{errors.inactive_down_rate}</FormErrorMessage>
                : <FormHelperText>The download and upload rate limits for a torrent to be considered active by the queuing mechanism. A torrent whose download rate is less than <Code>inactive_down_rate</Code> and whose upload rate is less than <Code>inactive_up_rate</Code> for <Code>auto_manage_startup</Code> seconds, is considered inactive, and another queued torrent may be started. This logic is disabled if <Code>dont_count_slow_torrents</Code> is false.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="inactive_up_rate">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Inactive upload rate</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.inactive_up_rate && touched.inactive_up_rate
                ? <FormErrorMessage>{errors.inactive_up_rate}</FormErrorMessage>
                : <FormHelperText>See <em>Inactive download rate</em>.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="auto_manage_prefer_seeds">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Auto manage prefer seeds</FormLabel>
            <Checkbox
              id={"auto_manage_prefer_seeds"}
              name={"auto_manage_prefer_seeds"}
              onChange={handleChange}
              isChecked={values.auto_manage_prefer_seeds}
            />
            {
              errors.auto_manage_prefer_seeds && touched.auto_manage_prefer_seeds
                ? <FormErrorMessage>{errors.auto_manage_prefer_seeds}</FormErrorMessage>
                : <FormHelperText>If true, prefer seeding torrents when determining which torrents to give active slots to. If false, give preference to downloading torrents</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="dont_count_slow_torrents">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Don't count slow torrents</FormLabel>
            <Checkbox
              id={"dont_count_slow_torrents"}
              name={"dont_count_slow_torrents"}
              onChange={handleChange}
              isChecked={values.dont_count_slow_torrents}
            />
            {
              errors.dont_count_slow_torrents && touched.dont_count_slow_torrents
                ? <FormErrorMessage>{errors.dont_count_slow_torrents}</FormErrorMessage>
                : <FormHelperText>If true, torrents without any payload transfers are not subject to the <Code>active_seeds</Code> and <Code>active_downloads</Code> limits. This is intended to make it more likely to utilize all available bandwidth, and avoid having torrents that don't transfer anything block the active slots.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="incoming_starts_queued_torrents">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Incoming starts queued torrents</FormLabel>
            <Checkbox
              id={"incoming_starts_queued_torrents"}
              name={"incoming_starts_queued_torrents"}
              onChange={handleChange}
              isChecked={values.incoming_starts_queued_torrents}
            />
            {
              errors.incoming_starts_queued_torrents && touched.incoming_starts_queued_torrents
                ? <FormErrorMessage>{errors.incoming_starts_queued_torrents}</FormErrorMessage>
                : <FormHelperText>If a torrent has been paused by the auto managed feature in libtorrent, i.e. the torrent is paused and auto managed, this feature affects whether or not it is automatically started on an incoming connection. The main reason to queue torrents, is not to make them unavailable, but to save on the overhead of announcing to the trackers, the DHT and to avoid spreading one's unchoke slots too thin. If a peer managed to find us, even though we're no in the torrent anymore, this setting can make us start the torrent and serve it.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
    </Box>
  )
}
