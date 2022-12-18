import { Checkbox } from "@chakra-ui/checkbox";
import { FormControl, FormLabel, FormErrorMessage, FormHelperText } from "@chakra-ui/form-control";
import { Input } from "@chakra-ui/input";
import { Box, Code, ListItem, UnorderedList } from "@chakra-ui/layout";
import { Select } from "@chakra-ui/select";
import { Field, useFormikContext } from "formik";

type IFormData = {
  allow_multiple_connections_per_ip: boolean;
  allowed_fast_set_size: number;
  announce_to_all_trackers: boolean;
  checking_mem_usage: number;
  choking_algorithm: number;
  connection_speed: number;
  connections_limit: number;
  inactivity_timeout: number;
  initial_picker_threshold: number;
  listen_queue_size: number;
  max_allowed_in_request_queue: number;
  max_failcount: number;
  max_out_request_queue: number;
  max_peer_recv_buffer_size: number;
  max_rejects: number;
  min_reconnect_time: number;
  mixed_mode_algorithm: number;
  peer_timeout: number;
  peer_turnover: number;
  peer_turnover_cutoff: number;
  peer_turnover_interval: number;
  predictive_piece_announce: number;
  rate_choker_initial_threshold: number;
  request_timeout: number;
  seed_choking_algorithm: number;
  send_not_sent_low_watermark: number;
  strict_end_game_mode: boolean;
  suggest_mode: boolean;
  torrent_connect_boost: number;
  unchoke_slots_limit: number;
  use_parole_mode: boolean;
  whole_pieces_threshold: boolean;
}

export default function NetworkSettingsTab() {
  const {
    errors,
    handleChange,
    touched,
    setFieldValue,
    values
  } = useFormikContext<IFormData>();

  type SettingProps = {
    desc: string | JSX.Element;
    field: keyof IFormData;
    title: string;
  }

  return (
    <Box>
      <Field name="connections_limit">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Connections limit</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.connections_limit && touched.connections_limit
              ? <FormErrorMessage>{errors.connections_limit}</FormErrorMessage>
              : <FormHelperText>A global limit on the number of connections opened. The number of connections is set to a hard minimum of at least two per torrent, so if you set a too low connections limit, and open too many torrents, the limit will not be met.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="connection_speed">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Connection speed</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.connection_speed && touched.connection_speed
              ? <FormErrorMessage>{errors.connection_speed}</FormErrorMessage>
              : <FormHelperText>The number of connection attempts that are made per second. If a number &lt; 0 is specified, it will default to 200 connections per second. If 0 is specified, it means don't make outgoing connections at all.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="request_timeout">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Request timeout</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.request_timeout && touched.request_timeout
              ? <FormErrorMessage>{errors.request_timeout}</FormErrorMessage>
              : <FormHelperText>The number of seconds one block (16 kiB) is expected to be received within. If it's not, the block is requested from a different peer.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="peer_timeout">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Peer timeout</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.peer_timeout && touched.peer_timeout
              ? <FormErrorMessage>{errors.peer_timeout}</FormErrorMessage>
              : <FormHelperText>The number of seconds the peer connection should wait (for any activity on the peer connection) before closing it due to time out. 120 seconds is specified in the protocol specification. After half the time out, a keep alive message is sent.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="peer_turnover">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Peer turnover</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.peer_turnover && touched.peer_turnover
              ? <FormErrorMessage>{errors.peer_turnover}</FormErrorMessage>
              : <FormHelperText>The percentage of peers to disconnect every turnover peer_turnover_interval (if we're at the peer limit), this is specified in percent when we are connected to more than limit * peer_turnover_cutoff peers disconnect peer_turnover fraction of the peers. It is specified in percent peer_turnover_interval is the interval (in seconds) between optimistic disconnects if the disconnects happen and how many peers are disconnected is controlled by peer_turnover and peer_turnover_cutoff</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="peer_turnover_cutoff">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Peer turnover cutoff</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.peer_turnover_cutoff && touched.peer_turnover_cutoff
              ? <FormErrorMessage>{errors.peer_turnover_cutoff}</FormErrorMessage>
              : <FormHelperText>See <em>Peer turnover</em>.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="peer_turnover_interval">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Peer turnover interval</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.peer_turnover_interval && touched.peer_turnover_interval
              ? <FormErrorMessage>{errors.peer_turnover_interval}</FormErrorMessage>
              : <FormHelperText>See <em>Peer turnover</em>.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="inactivity_timeout">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Inactivity timeout</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.inactivity_timeout && touched.inactivity_timeout
              ? <FormErrorMessage>{errors.inactivity_timeout}</FormErrorMessage>
              : <FormHelperText>If a peer is uninteresting and uninterested for longer than this number of seconds, it will be disconnected.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_allowed_in_request_queue">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max allowed in request queue</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_allowed_in_request_queue && touched.max_allowed_in_request_queue
              ? <FormErrorMessage>{errors.max_allowed_in_request_queue}</FormErrorMessage>
              : <FormHelperText>The number of outstanding block requests a peer is allowed to queue up in the client. If a peer sends more requests than this (before the first one has been sent) the last request will be dropped. the higher this is, the faster upload speeds the client can get to a single peer.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_out_request_queue">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max out request queue</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_out_request_queue && touched.max_out_request_queue
              ? <FormErrorMessage>{errors.max_out_request_queue}</FormErrorMessage>
              : <FormHelperText>The maximum number of outstanding requests to send to a peer. This limit takes precedence over <Code>request_queue_time</Code>. i.e. no matter the download speed, the number of outstanding requests will never exceed this limit.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="choking_algorithm">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Choking algorithm</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("choking_algorithm", parseInt(e.target.value))
              }}
            >
              <option value={0}>fixed_slots_choker</option>
              <option value={1}>rate_based_choker</option>
            </Select>
            {
              errors.choking_algorithm && touched.choking_algorithm
              ? <FormErrorMessage>{errors.choking_algorithm}</FormErrorMessage>
              : <FormHelperText>
                  Specifies which algorithm to use to determine how many peers to unchoke. The unchoking algorithm for downloading torrents is always "tit-for-tat", i.e. the peers we download the fastest from are unchoked.
                  <UnorderedList mt={2}>
                    <ListItem mb={2}><Code>fixed_slots_choker</Code> - This is the traditional choker with a fixed number of unchoke slots (as specified by <Code>unchoke_slots_limit</Code>).</ListItem>
                    <ListItem><Code>rate_based_choker</Code> - This opens up unchoke slots based on the upload rate achieved to peers. The more slots that are opened, the marginal upload rate required to open up another slot increases. Configure the initial threshold with <Code>rate_choker_initial_threshold</Code>.</ListItem>
                  </UnorderedList>
                </FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="seed_choking_algorithm">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Seed choking algorithm</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("seed_choking_algorithm", parseInt(e.target.value))
              }}
            >
              <option value={0}>round_robin</option>
              <option value={1}>fastest_upload</option>
              <option value={2}>anti_leech</option>
            </Select>
            {
              errors.seed_choking_algorithm && touched.seed_choking_algorithm
              ? <FormErrorMessage>{errors.seed_choking_algorithm}</FormErrorMessage>
              : <FormHelperText>
                  Controls the seeding unchoke behavior. i.e. How we select which peers to unchoke for seeding torrents. Since a seeding torrent isn't downloading anything, the tit-for-tat mechanism cannot be used.
                  <UnorderedList mt={2}>
                    <ListItem mb={2}><Code>round_robin</Code> - This distributes the upload bandwidth uniformly and fairly. It minimizes the ability for a peer to download everything without redistributing it.</ListItem>
                    <ListItem mb={2}><Code>fastest_upload</Code> - Unchokes the peers we can send to the fastest. This might be a bit more reliable in utilizing all available capacity.</ListItem>
                    <ListItem><Code>anti_leech</Code> - prioritizes peers who have just started or are just about to finish the download. The intention is to force peers in the middle of the download to trade with each other. This does not just take into account the pieces a peer is reporting having downloaded, but also the pieces we have sent to it.</ListItem>
                  </UnorderedList>
                </FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="unchoke_slots_limit">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Unchoke slots limit</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.unchoke_slots_limit && touched.unchoke_slots_limit
              ? <FormErrorMessage>{errors.unchoke_slots_limit}</FormErrorMessage>
              : <FormHelperText>The max number of unchoked peers in the session. The number of unchoke slots may be ignored depending on what <Code>choking_algorithm</Code> is set to. Setting this limit to -1 means unlimited, i.e. all peers will always be unchoked.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="rate_choker_initial_threshold">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Rate choker initial threshold</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.rate_choker_initial_threshold && touched.rate_choker_initial_threshold
              ? <FormErrorMessage>{errors.rate_choker_initial_threshold}</FormErrorMessage>
              : <FormHelperText>The rate based choker compares the upload rate to peers against a threshold that increases proportionally by its size for every peer it visits, visiting peers in decreasing upload rate. The number of upload slots is determined by the number of peers whose upload rate exceeds the threshold. This option sets the start value for this threshold. A higher value leads to fewer unchoke slots, a lower value leads to more.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="allow_multiple_connections_per_ip">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Allow multiple connections per IP</FormLabel>
            <Checkbox
              id={"allow_multiple_connections_per_ip"}
              name={"allow_multiple_connections_per_ip"}
              onChange={handleChange}
              isChecked={values.allow_multiple_connections_per_ip}
            />
            {
              errors.allow_multiple_connections_per_ip && touched.allow_multiple_connections_per_ip
                ? <FormErrorMessage>{errors.allow_multiple_connections_per_ip}</FormErrorMessage>
                : <FormHelperText>Determines if connections from the same IP address as existing connections should be rejected or not. Rejecting multiple connections from the same IP address will prevent abusive behavior by peers. The logic for determining whether connections are to the same peer is more complicated with this enabled, and more likely to fail in some edge cases. It is not recommended to enable this feature.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="use_parole_mode">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Use parole mode</FormLabel>
            <Checkbox
              id={"use_parole_mode"}
              name={"use_parole_mode"}
              onChange={handleChange}
              isChecked={values.use_parole_mode}
            />
            {
              errors.use_parole_mode && touched.use_parole_mode
                ? <FormErrorMessage>{errors.use_parole_mode}</FormErrorMessage>
                : <FormHelperText>Specifies if parole mode should be used. Parole mode means that peers that participate in pieces that fail the hash check are put in a mode where they are only allowed to download whole pieces. If the whole piece a peer in parole mode fails the hash check, it is banned. If a peer participates in a piece that passes the hash check, it is taken out of parole mode.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="announce_to_all_trackers">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Announce to all trackers</FormLabel>
            <Checkbox
              id={"announce_to_all_trackers"}
              name={"announce_to_all_trackers"}
              onChange={handleChange}
              isChecked={values.announce_to_all_trackers}
            />
            {
              errors.announce_to_all_trackers && touched.announce_to_all_trackers
                ? <FormErrorMessage>{errors.announce_to_all_trackers}</FormErrorMessage>
                : <FormHelperText>Controls how multi tracker torrents are treated. If this is set to true, all trackers in the same tier are announced to in parallel. If all trackers in tier 0 fails, all trackers in tier 1 are announced as well. If it's set to false, the behavior is as defined by the multi tracker specification.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="strict_end_game_mode">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Strict end game mode</FormLabel>
            <Checkbox
              id={"strict_end_game_mode"}
              name={"strict_end_game_mode"}
              onChange={handleChange}
              isChecked={values.strict_end_game_mode}
            />
            {
              errors.strict_end_game_mode && touched.strict_end_game_mode
                ? <FormErrorMessage>{errors.strict_end_game_mode}</FormErrorMessage>
                : <FormHelperText>Controls when a block may be requested twice. If this is <Code>true</Code>, a block may only be requested twice when there's at least one request to every piece that's left to download in the torrent. This may slow down progress on some pieces sometimes, but it may also avoid downloading a lot of redundant bytes. If this is <Code>false</Code>, libtorrent attempts to use each peer connection to its max, by always requesting something, even if it means requesting something that has been requested from another peer already.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="whole_pieces_threshold">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Whole pieces threshold</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.whole_pieces_threshold && touched.whole_pieces_threshold
              ? <FormErrorMessage>{errors.whole_pieces_threshold}</FormErrorMessage>
              : <FormHelperText>If a whole piece can be downloaded in this number of seconds, or less, the peer_connection will prefer to request whole pieces at a time from this peer. The benefit of this is to better utilize disk caches by doing localized accesses and also to make it easier to identify bad peers if a piece fails the hash check.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_failcount">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max failcount</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_failcount && touched.max_failcount
              ? <FormErrorMessage>{errors.max_failcount}</FormErrorMessage>
              : <FormHelperText>The maximum times we try to connect to a peer before stop connecting again. If a peer succeeds, the failure counter is reset. If a peer is retrieved from a peer source (other than DHT) the failcount is decremented by one, allowing another try.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="min_reconnect_time">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Min. reconnect time</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.min_reconnect_time && touched.min_reconnect_time
              ? <FormErrorMessage>{errors.min_reconnect_time}</FormErrorMessage>
              : <FormHelperText>The number of seconds to wait to reconnect to a peer. this time is multiplied with the failcount.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="initial_picker_threshold">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Initial picker threshold</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.initial_picker_threshold && touched.initial_picker_threshold
              ? <FormErrorMessage>{errors.initial_picker_threshold}</FormErrorMessage>
              : <FormHelperText>The number of pieces we need before we switch to rarest first picking. The first <Code>initial_picker_threshold</Code> pieces in any torrent are picked at random , the following pieces are picked in rarest first order.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="allowed_fast_set_size">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Allowed fast set size</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.allowed_fast_set_size && touched.allowed_fast_set_size
              ? <FormErrorMessage>{errors.allowed_fast_set_size}</FormErrorMessage>
              : <FormHelperText>The number of allowed pieces to send to peers that supports the fast extensions</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="suggest_mode">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Suggest mode</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("suggest_mode", parseInt(e.target.value))
              }}
            >
              <option value={0}>no_piece_suggestions</option>
              <option value={1}>suggest_read_cache</option>
            </Select>
            {
              errors.suggest_mode && touched.suggest_mode
              ? <FormErrorMessage>{errors.suggest_mode}</FormErrorMessage>
              : <FormHelperText>
                  Controls whether or not libtorrent will send out suggest messages to create a bias of its peers to request certain pieces.
                  <UnorderedList mt={2}>
                    <ListItem mb={2}><Code>no_piece_suggestions</Code> - will not send out suggest messages.</ListItem>
                    <ListItem><Code>suggest_read_cache</Code> - will send out suggest messages for the most recent pieces that are in the read cache.</ListItem>
                  </UnorderedList>
                </FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_rejects">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max rejects</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_rejects && touched.max_rejects
              ? <FormErrorMessage>{errors.max_rejects}</FormErrorMessage>
              : <FormHelperText>The number of piece requests we will reject in a row while a peer is choked before the peer is considered abusive and is disconnected.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="max_peer_recv_buffer_size">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Max peer recieve buffer size</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.max_peer_recv_buffer_size && touched.max_peer_recv_buffer_size
              ? <FormErrorMessage>{errors.max_peer_recv_buffer_size}</FormErrorMessage>
              : <FormHelperText>The max number of bytes a single peer connection's receive buffer is allowed to grow to.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="mixed_mode_algorithm">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Mixed mode algorithm</FormLabel>
            <Select
              {...w.field}
              onChange={e => {
                setFieldValue("mixed_mode_algorithm", parseInt(e.target.value))
              }}
            >
              <option value={0}>prefer_tcp</option>
              <option value={1}>peer_proportional</option>
            </Select>
            {
              errors.mixed_mode_algorithm && touched.mixed_mode_algorithm
              ? <FormErrorMessage>{errors.mixed_mode_algorithm}</FormErrorMessage>
              : <FormHelperText>Determines how to treat TCP connections when there are uTP connections. Since uTP is designed to yield to TCP, there's an inherent problem when using swarms that have both TCP and uTP connections. If nothing is done, uTP connections would often be starved out for bandwidth by the TCP connections. This mode is <Code>prefer_tcp</Code>. The <Code>peer_proportional</Code> mode simply looks at the current throughput and rate limits all TCP connections to their proportional share based on how many of the connections are TCP. This works best if uTP connections are not rate limited by the global rate limiter (which they aren't by default).</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="listen_queue_size">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Listen queue size</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.listen_queue_size && touched.listen_queue_size
              ? <FormErrorMessage>{errors.listen_queue_size}</FormErrorMessage>
              : <FormHelperText>The value passed in to listen() for the listen socket. It is the number of outstanding incoming connections to queue up while we're not actively waiting for a connection to be accepted. 5 should be sufficient for any normal client. If this is a high performance server which expects to receive a lot of connections, or used in a simulator or test, it might make sense to raise this number. It will not take affect until the <Code>listen_interfaces</Code> settings is updated or Porla is restarted.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="torrent_connect_boost">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Torrent connect boost</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.torrent_connect_boost && touched.torrent_connect_boost
              ? <FormErrorMessage>{errors.torrent_connect_boost}</FormErrorMessage>
              : <FormHelperText>The number of peers to try to connect to immediately when the first tracker response is received for a torrent. This is a boost to given to new torrents to accelerate them starting up. The normal connect scheduler is run once every second, this allows peers to be connected immediately instead of waiting for the session tick to trigger connections. This may not be set higher than 255.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="checking_mem_usage">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Checking memory usage</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.checking_mem_usage && touched.checking_mem_usage
              ? <FormErrorMessage>{errors.checking_mem_usage}</FormErrorMessage>
              : <FormHelperText>The number of blocks to keep outstanding at any given time when checking torrents. Higher numbers give faster re-checks but uses more memory. Specified in number of 16 kiB blocks.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="predictive_piece_announce">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Predictive piece announce</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.predictive_piece_announce && touched.predictive_piece_announce
              ? <FormErrorMessage>{errors.predictive_piece_announce}</FormErrorMessage>
              : <FormHelperText>If set to &gt; 0, pieces will be announced to other peers before they are fully downloaded (and before they are hash checked). The intention is to gain 1.5 potential round trip times per downloaded piece. When non-zero, this indicates how many milliseconds in advance pieces should be announced, before they are expected to be completed.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
      <Field name="send_not_sent_low_watermark">
        {(w: any) => (
          <FormControl mb={3}>
            <FormLabel>Send not-sent low watermark buffer</FormLabel>
            <Input
              {...w.field}
              type={"number"}
            />
            {
              errors.send_not_sent_low_watermark && touched.send_not_sent_low_watermark
              ? <FormErrorMessage>{errors.send_not_sent_low_watermark}</FormErrorMessage>
              : <FormHelperText>Specify the not-sent low watermark for socket send buffers. This corresponds to the, Linux-specific, <Code>TCP_NOTSENT_LOWAT</Code> TCP socket option.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
    </Box>
  )
}
