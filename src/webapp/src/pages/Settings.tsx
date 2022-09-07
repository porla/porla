import React from "react";
import { useTranslation } from "react-i18next";
import { Button, Checkbox, FormControl, FormHelperText, FormLabel, Input, Select } from "@chakra-ui/react";
import { Field, Formik } from "formik";
import { trpc } from "../utils/trpc";

import Card from "../components/Card";
import Loading from "../components/Loading";

export default function Settings() {
  const { t } = useTranslation();

  const config = trpc.useQuery(["config.get", [
    "default_save_path",
    "proxy_type",
    "proxy_hostname",
    "proxy_port"
  ]]);

  const setConfig = trpc.useMutation(["config.set"]);

  if (!config.data) {
    return <Loading />
  }

  return (
    <>
      <Formik
        initialValues={{
          default_save_path: config.data.default_save_path || "",
          proxy_type: config.data.proxy_type || 0,
          proxy_hostname: config.data.proxy_hostname || "",
          proxy_port: config.data.proxy_port || 1080
        }}
        onSubmit={async (values) => {
          await setConfig.mutateAsync({
            ...values,
            // force number for certain properties
            proxy_type: parseInt(values.proxy_type)
          });
        }}
      >
        {({ handleChange, handleSubmit, values }) => (
          <form onSubmit={handleSubmit}>
            <Card heading={t('general')}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel htmlFor="default_save_path">{t('default_save_path')}</FormLabel>
                <Field
                  as={Input}
                  id="default_save_path"
                  name="default_save_path"
                  placeholder="/mnt/downloads"
                  type="text"
                />
                <FormHelperText>{t('default_save_path_helper')}</FormHelperText>
              </FormControl>
            </Card>

            <Card heading={t('proxy')} mt={5}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t('type')}</FormLabel>
                <Select
                  id="proxy_type"
                  name="proxy_type"
                  onChange={handleChange}
                  value={values.proxy_type}
                >
                  <option value="0">{t('none')}</option>
                  <option value="5">SOCKS5</option>
                </Select>
              </FormControl>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t('proxy_host')}</FormLabel>
                <Field
                  as={Input}
                  id="proxy_hostname"
                  isDisabled={values.proxy_type==0}
                  name="proxy_hostname"
                  placeholder="10.64.0.1"
                  type="text"
                />
                <FormHelperText>{t('proxy_host_helper')}</FormHelperText>
              </FormControl>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>Port</FormLabel>
                <Field
                  as={Input}
                  id="proxy_port"
                  isDisabled={values.proxy_type==0}
                  name="proxy_port"
                  placeholder="1080"
                  type="number"
                />
                <FormHelperText>{t('proxy_port_helper')}</FormHelperText>
              </FormControl>
            </Card>

            <Button
              colorScheme={"purple"}
              marginTop={5}
              type="submit"
            >
              {t('save_settings')}
            </Button>
          </form>
        )}
      </Formik>
    </>
  );
}
