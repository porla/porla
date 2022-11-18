import { Box, FormControl, FormErrorMessage, FormHelperText, FormLabel, Textarea } from "@chakra-ui/react";
import { Field, useFormikContext } from "formik";

type IFormData = {
  listen_interfaces: string;
}

export default function GeneralSettingsTab() {
  const {
    errors,
    setFieldValue,
    touched,
    values
  } = useFormikContext<IFormData>();

  return (
    <Box>
      <Field name="listen_interfaces">
        {(w: any) => (
          <FormControl isInvalid={!!errors.listen_interfaces && touched.listen_interfaces}>
            <FormLabel>Listen interfaces</FormLabel>
            <Textarea
              {...w.field}
              value={values.listen_interfaces.replace(",", "\n")}
              rows={Math.max(3, values.listen_interfaces.split(",").length)}
              onChange={e => setFieldValue("listen_interfaces", e.target.value)}
            />
            {
              errors.listen_interfaces && touched.listen_interfaces
                ? <FormErrorMessage>{errors.listen_interfaces}</FormErrorMessage>
                : <FormHelperText>Listen interfaces to use for BitTorrent connections. Specified as IP addresses or device names with ports. One per line.</FormHelperText>
            }
          </FormControl>
        )}
      </Field>
    </Box>
  )
}
