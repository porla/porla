import React from "react";
import { useNavigate } from "react-router-dom";
import { useTranslation } from "react-i18next";
import { Formik, Field } from "formik";
import { trpc } from "../utils/trpc";
import { Box, Button, FormControl, FormHelperText, FormLabel, Heading, Input, useToast } from "@chakra-ui/react";
import Loading from "../components/Loading";
import Card from "../components/Card";

function Add() {
  const { t } = useTranslation();
  const toast = useToast();
  const navigate = useNavigate();
  const config = trpc.useQuery(["config.get", ["default_save_path"]]);
  const addTorrent = trpc.useMutation(["torrents.add"]);

  if (!config.data) {
    return <Loading />
  }

  return (
    <>
      <Card heading={t("add_torrent")}>
        <Formik
          initialValues={{
            magnet_link: "",
            save_path: config.data.default_save_path || "",
          }}
          onSubmit={async (values) => {
            try {
              await addTorrent.mutateAsync(values);
              navigate('/torrents');
              toast({
                title: t("success"),
                description: t("add_torrent_success"),
                status: "success",
                isClosable: true,
              })
            } catch (error) {
              toast({
                title: t("error"),
                description: t("add_torrent_error"),
                status: "error",
                isClosable: true,
              })
            }
          }}
        >
          {({ handleSubmit }) => (
            <form onSubmit={handleSubmit}>
              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t("magnet_link")}</FormLabel>
                <Field
                  as={Input}
                  id="magnet_link"
                  name="magnet_link"
                  placeholder="magnet:?xt=urn:btih: ..."
                  type="text"
                />
                <FormHelperText>{t("magnet_link_helper")}</FormHelperText>
              </FormControl>

              <FormControl
                marginBottom={3}
              >
                <FormLabel>{t("save_path")}</FormLabel>
                <Field
                  as={Input}
                  id="save_path"
                  name="save_path"
                  placeholder={t("default_save_path_placeholder")}
                  type="text"
                />
                <FormHelperText>{t("save_path_helper")}</FormHelperText>
              </FormControl>

              <Button
                colorScheme={"purple"}
                marginTop={3}
                size="sm"
                type="submit"
              >
                {t("add_torrent")}
              </Button>
            </form>
          )}
        </Formik>
      </Card>
    </>
  );
}

export default Add;
