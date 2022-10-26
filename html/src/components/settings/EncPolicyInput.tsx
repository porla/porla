import { Select } from "@chakra-ui/react";

export default function EncPolicyInput(props: { value: number }) {
  return (
    <Select isReadOnly value={props.value}>
      <option value="0">pe_forced</option>
      <option value="1">pe_enabled</option>
      <option value="2">pe_disabled</option>
    </Select>
  )
}
