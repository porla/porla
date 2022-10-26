import { Select } from "@chakra-ui/react";

export default function IoBufferModeInput(props: { value: number }) {
  return (
    <Select isReadOnly value={props.value}>
      <option value="0">enable_os_cache</option>
      <option value="2">disable_os_cache</option>
      <option value="3">write_through</option>
    </Select>
  )
}
