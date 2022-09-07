import React from "react";
import { useTranslation } from "react-i18next";
import { Formik, Field } from "formik";
import { trpc } from "../utils/trpc";
import { Box, Button, FormControl, FormHelperText, FormLabel, Heading, Input } from "@chakra-ui/react";
import Loading from "../components/Loading";

function Add() {
  const { t } = useTranslation();

  const config = trpc.useQuery(["config.get", ["default_save_path"]]);
  const addTorrent = trpc.useMutation(["torrents.add"]);

  if (!config.data) {
    return <Loading />
  }

  return (
    <>
      <Box
        backgroundColor={"#fff"}
        border="1px solid #f0f0f0"
        borderRadius={4}
        padding={3}
      >
        <Heading
          color={"#444"}
          marginBottom={3}
          size="md"
        >
          {t('add_torrent')}
        </Heading>
        <Formik
          initialValues={{
            magnet_link: "",
            save_path: config.data.default_save_path || ""
          }}
          onSubmit={async (values) => {
            await addTorrent.mutateAsync(values)
          }}
        >
          {({ handleSubmit }) => (
            <form onSubmit={handleSubmit}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t('magnet_link')}</FormLabel>
                <Field
                  as={Input}
                  id="magnet_link"
                  name="magnet_link"
                  placeholder="magnet:?xt=urn:btih: ..."
                  type="text"
                />
                <FormHelperText>{t('magnet_link_helper')}</FormHelperText>
              </FormControl>

              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t('save_path')}</FormLabel>
                <Field
                  as={Input}
                  id="save_path"
                  name="save_path"
                  placeholder="/mnt/downloads"
                  type="text"
                />
                <FormHelperText>{t('save_path_helper')}</FormHelperText>
              </FormControl>

              <Button
                colorScheme={"purple"}
                marginTop={3}
                size="sm"
                type="submit"
              >
                {t('add_torrent')}
              </Button>
            </form>
          )}
        </Formik>
      </Box>
    </>
  );
}

export default Add;
